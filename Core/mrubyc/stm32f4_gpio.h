#ifndef STM32F4_GPIO_H
#define STM32F4_GPIO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/*!
  physical pin describer.
*/
typedef struct PIN_HANDLE {
  uint8_t port;		// A=1,B=2,..,G=7
  uint8_t num;		// 0..15
} PIN_HANDLE;

#define GPIO_IN			0x01
#define GPIO_OUT		0x02
#define GPIO_ANALOG		0x04
#define GPIO_HIGH_Z		0x08
#define GPIO_PULL_UP		0x10
#define GPIO_PULL_DOWN		0x20
#define GPIO_OPEN_DRAIN		0x40

struct RObject;
int gpio_set_pin_handle( PIN_HANDLE *pin_handle, const struct RObject *val );
int gpio_setmode( const PIN_HANDLE *pin, unsigned int mode );
int gpio_setmode_pwm( const PIN_HANDLE *pin, int ch );
void mrbc_init_class_gpio( void );


#ifdef __cplusplus
}
#endif
#endif
