#include "main.h"
#include "../mrubyc_src/mrubyc.h"

#include <string.h>

#define I2C_TIMEOUT_ms 3000

extern I2C_HandleTypeDef hi2c1;

//----------------------------------------------------------------
/*! make output buffer
*/
uint8_t * make_output_buffer(mrb_vm *vm, mrb_value v[], int argc,
			     int start_idx, int *ret_bufsiz)
{
  uint8_t *ret = 0;

  // calc temporary buffer size.
  int bufsiz = 0;
  for( int i = start_idx; i <= argc; i++ ) {
    switch( v[i].tt ) {
    case MRBC_TT_INTEGER:
      bufsiz += 1;
      break;

    case MRBC_TT_STRING:
      bufsiz += mrbc_string_size(&v[i]);
      break;

    case MRBC_TT_ARRAY:
      bufsiz += mrbc_array_size(&v[i]);
      break;

    default:
      goto ERROR_PARAM;
    }
  }
  *ret_bufsiz = bufsiz;
  if( bufsiz == 0 ) return ret;

  // alloc buffer and copy data
  ret = mrbc_alloc(vm, bufsiz);
  uint8_t *pbuf = ret;
  for( int i = start_idx; i <= argc; i++ ) {
    switch( v[i].tt ) {
    case MRBC_TT_INTEGER:
      *pbuf++ = mrbc_integer(v[i]);
      break;

    case MRBC_TT_STRING:
      memcpy( pbuf, mrbc_string_cstr(&v[i]), mrbc_string_size(&v[i]) );
      pbuf += mrbc_string_size(&v[i]);
      break;

    case MRBC_TT_ARRAY: {
      for( int j = 0; j < mrbc_array_size(&v[i]); j++ ) {
	mrbc_value val = mrbc_array_get(&v[i], j);
	if( val.tt != MRBC_TT_INTEGER ) goto ERROR_PARAM;
	*pbuf++ = mrbc_integer(val);
      }
    } break;

    default:
      //
    }
  }
  return ret;


 ERROR_PARAM:
  mrbc_raise(vm, MRBC_CLASS(ArgumentError), "Output parameter error.");
  if( ret != 0 ) {
    mrbc_free( vm, ret );
  }

  return 0;
}


//================================================================
/*! read transaction.

  (mruby usage)
  s = i2c.read( i2c_adrs_7, read_bytes, *param )
  s.getbyte(n)  # bytes

  i2c_adrs_7 = Integer
  read_byres = Integer
  *param     = (option)

  (I2C Sequence)
  S - adrs W A - [param A...] - Sr - adrs R A - data_1 A... data_n A|N - [P]
    S : Start condition
    P : Stop condition
    Sr: Repeated start condition
    A : Ack
    N : Nack
*/
static void c_i2c_read(mrb_vm *vm, mrb_value v[], int argc)
{
  uint8_t *buf = 0;
  int bufsiz = 0;
  mrbc_value ret = mrbc_nil_value();

  // Get parameter
  if( argc < 2 ) goto ERROR_PARAM;
  if( v[1].tt != MRBC_TT_INTEGER ) goto ERROR_PARAM;
  int i2c_adrs_7 = mrbc_integer(v[1]);

  if( v[2].tt != MRBC_TT_INTEGER ) goto ERROR_PARAM;
  int read_bytes = mrbc_integer(v[2]);
  if( read_bytes < 0 ) goto ERROR_PARAM;

  if( argc > 2 ) {
    buf = make_output_buffer( vm, v, argc, 3, &bufsiz );
    if( !buf ) goto RETURN;
    if( bufsiz > 2 ) {
      mrbc_raise(vm, 0, "i2c#read: output parameter must be less than 2 bytes.");
      goto RETURN;
    }
  }

  // Start I2C communication
  ret = mrbc_string_new(vm, 0, read_bytes);
  uint8_t *p = (uint8_t *)mrbc_string_cstr(&ret);
  HAL_StatusTypeDef sts;

  if( buf && bufsiz == 1 ) {
    sts = HAL_I2C_Mem_Read( &hi2c1, i2c_adrs_7 << 1, buf[0],
		I2C_MEMADD_SIZE_8BIT, p, read_bytes, I2C_TIMEOUT_ms );

  } else if( buf && bufsiz == 2 ) {
    sts = HAL_I2C_Mem_Read( &hi2c1, i2c_adrs_7 << 1, buf[0] << 8 | buf[1],
		I2C_MEMADD_SIZE_16BIT, p, read_bytes, I2C_TIMEOUT_ms );

  } else {
    sts = HAL_I2C_Master_Receive( &hi2c1, i2c_adrs_7 << 1,
				  p, read_bytes, I2C_TIMEOUT_ms );
  }
  if( sts != HAL_OK ) {
    mrbc_raisef(vm, 0, "i2c#read: HAL layer error (status code %d)", sts);
  }
  goto RETURN;


 ERROR_PARAM:
  mrbc_raise(vm, MRBC_CLASS(ArgumentError), "i2c#read: parameter error.");

 RETURN:
  if( buf ) mrbc_free( vm, buf );
  SET_RETURN(ret);
}


//================================================================
/*! write transaction.

  (mruby usage)
  i2c.write( i2c_adrs_7, write_data, ... )

  i2c_adrs_7 = Integer
  write_data = String, Integer, or Array<Integer>

  (I2C Sequence)
  S - ADRS W A - data1 A... - P

    S : Start condition
    P : Stop condition
    A : Ack
*/
static void c_i2c_write(mrb_vm *vm, mrb_value v[], int argc)
{
  uint8_t *buf = 0;
  int bufsiz = 0;

  // Get parameter
  if( argc < 1 ) goto ERROR_PARAM;
  if( v[1].tt != MRBC_TT_INTEGER ) goto ERROR_PARAM;
  int i2c_adrs_7 = mrbc_integer(v[1]);

  buf = make_output_buffer( vm, v, argc, 2, &bufsiz );
  if( !buf ) goto RETURN;

  // Start I2C communication
  HAL_StatusTypeDef sts;
  sts = HAL_I2C_Master_Transmit( &hi2c1, i2c_adrs_7 << 1,
				 buf, bufsiz, I2C_TIMEOUT_ms );
  mrbc_free( vm, buf );

  if( sts != HAL_OK ) {
    mrbc_raisef(vm, 0, "i2c#write: HAL layer error (status code %d)", sts);
  }
  goto RETURN;


 ERROR_PARAM:
  mrbc_raise(vm, MRBC_CLASS(ArgumentError), "i2c#write: parameter error.");

 RETURN:
  SET_RETURN( mrbc_integer_value(bufsiz) );
}


//================================================================
/*! initialize
*/
void mrbc_init_class_i2c(void)
{
  mrbc_class *cls = mrbc_define_class(0, "I2C", 0);

  mrbc_define_method(0, cls, "read", c_i2c_read);
  mrbc_define_method(0, cls, "write", c_i2c_write);
}
