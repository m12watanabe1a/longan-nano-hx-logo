#include "lcd/lcd.h"
#include "systick.h"
#include <stdio.h>

#define LED_PIN GPIO_PIN_13
#define LED_GPIO_PORT GPIOC
#define LED_GPIO_CLK RCU_GPIOC

void longan_led_init()
{
  rcu_periph_clock_enable(LED_GPIO_CLK);
  gpio_init(LED_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, LED_PIN);

  GPIO_BC(LED_GPIO_PORT) = LED_PIN;
}

void longan_led_on()
{
  GPIO_BC(LED_GPIO_PORT) = LED_PIN;
}

void longan_led_off()
{
  GPIO_BOP(LED_GPIO_PORT) = LED_PIN;
}

void longan_oled_init()
{
  LCD_Init();
  BACK_COLOR = GRAY;
  LCD_Clear(BACK_COLOR);
}

int main(void)
{
  longan_led_init();
  longan_oled_init();
  LCD_ShowLogo();
  while (1)
  {
  }
  return 0;
}