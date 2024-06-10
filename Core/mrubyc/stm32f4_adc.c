#include "main.h"
#include "../mrubyc_src/mrubyc.h"
#include "stm32f4_gpio.h"


extern ADC_HandleTypeDef hadc1;


/*! ADC handle
*/
typedef struct ADC_HANDLE {
  PIN_HANDLE pin;
  uint32_t channel;
} ADC_HANDLE;

/*
  Pin assign vs ADC channel table.
*/
static ADC_HANDLE const adc_handle_[] = {
  //                               GPIO  ADC ch.  silk
  {{1, 0}, ADC_CHANNEL_0},	// PA0   0        A0
  {{1, 1}, ADC_CHANNEL_1},	// PA1   1        A1
  {{1, 4}, ADC_CHANNEL_4},	// PA1   1        A2
  {{2, 0}, ADC_CHANNEL_8},	// PB0   8        A3
  {{3, 1}, ADC_CHANNEL_11},	// PC1   11       A4
  {{3, 0}, ADC_CHANNEL_10},	// PC0   10       A5
};


/*! constructor

  adc1 = ADC.new("PA1")
*/
static void c_adc_new(mrbc_vm *vm, mrbc_value v[], int argc)
{
  if( argc != 1 ) goto ERROR_RETURN;

  PIN_HANDLE pin;
  if( gpio_set_pin_handle( &pin, &v[1] ) != 0 ) goto ERROR_RETURN;

  // find ADC channel from adc_handle_ table.
  static const int NUM = sizeof(adc_handle_)/sizeof(ADC_HANDLE);
  int i;
  for( i = 0; i < NUM; i++ ) {
    if( (adc_handle_[i].pin.port == pin.port) &&
	(adc_handle_[i].pin.num == pin.num) ) break;
  }
  if( i == NUM ) goto ERROR_RETURN;

  // allocate instance with ADC_HANDLE table pointer.
  v[0] = mrbc_instance_new(vm, v[0].cls, sizeof(ADC_HANDLE *));
  *(const ADC_HANDLE **)(v[0].instance->data) = &adc_handle_[i];

  // set pin to analog input
  gpio_setmode( &pin, GPIO_ANALOG|GPIO_IN );
  return;

 ERROR_RETURN:
  mrbc_raise(vm, MRBC_CLASS(ArgumentError), "ADC initialize.");
}


static uint32_t read_sub(mrbc_vm *vm, mrbc_value v[], int argc)
{
  ADC_HANDLE *hndl = *(ADC_HANDLE **)v[0].instance->data;
  ADC_ChannelConfTypeDef sConfig = {
    .Channel = hndl->channel,
    .Rank = 1,
    .SamplingTime = ADC_SAMPLETIME_3CYCLES,
  };
  if( HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK ) return 0;

  HAL_ADC_Start(&hadc1);
  if( HAL_ADC_PollForConversion(&hadc1, 1000) != HAL_OK ) return 0;

  return HAL_ADC_GetValue(&hadc1);
}


/*! read_voltage

  adc1.read_voltage() -> Float
*/
static void c_adc_read_voltage(mrbc_vm *vm, mrbc_value v[], int argc)
{
  uint32_t raw_val = read_sub( vm, v, argc );

  SET_FLOAT_RETURN( raw_val * 3.3 / 4095 );
}


/*! read_raw

  adc1.read_raw() -> Integer
*/
static void c_adc_read_raw(mrbc_vm *vm, mrbc_value v[], int argc)
{
  uint32_t raw_val = read_sub( vm, v, argc );

  SET_INT_RETURN( raw_val );
}


/*! Initializer
*/
void mrbc_init_class_adc(void)
{
  mrbc_class *adc = mrbc_define_class(0, "ADC", 0);

  mrbc_define_method(0, adc, "new", c_adc_new);
  mrbc_define_method(0, adc, "read_voltage", c_adc_read_voltage);
  mrbc_define_method(0, adc, "read", c_adc_read_voltage);
  mrbc_define_method(0, adc, "read_raw", c_adc_read_raw);
}
