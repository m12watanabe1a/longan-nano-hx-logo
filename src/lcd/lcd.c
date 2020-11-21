#include "lcd/lcd.h"
#include "lcd/logo.h"
#include "lcd/oledfont.h"

u16 BACK_COLOR; // back ground color

void LCD_Write_Bus(u8 dat)
{
  OLED_CS_Clr();
#if SPI0_CFG == 1
  OLED_CS_Clr();
  while (RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_TBE))
    ;
  spi_i2s_data_transmit(SPI0, dat);
  while (RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_RBNE))
    ;
  spi_i2s_data_receive(SPI0);

  OLED_CS_Set();
#elif SPI0_CFG == 2
  spi_dma_enable(SPI0, SPI_DMA_TRANSMIT);
#else
  u8 i;
  OLED_CS_Clr();
  for (i = 0; i < 8; i++)
  {
    OLED_SCLK_Clr();
    if (dat & 0x80)
      OLED_SDIN_Set();
    else
      OLED_SDIN_Clr();
    OLED_SCLK_Set();
    dat <<= 1;
  }
  OLED_CS_Set();
#endif
}

void LCD_WR_DATA8(u8 dat)
{
  OLED_DC_Set();
  LCD_Write_Bus(dat);
}

void LCD_WR_DATA(u16 dat)
{
  OLED_DC_Set();
  LCD_Write_Bus(dat >> 8);
  LCD_Write_Bus(dat);
}

void LCD_WR_REG(u8 dat)
{
  OLED_DC_Clr();
  LCD_Write_Bus(dat);
}

void LCD_Address_Set(u16 x1, u16 y1, u16 x2, u16 y2)
{
  if (USE_HORIZONTAL == 0)
  {
    LCD_WR_REG(0x2a);
    LCD_WR_DATA(x1 + 26);
    LCD_WR_DATA(x2 + 26);
    LCD_WR_REG(0x2b);
    LCD_WR_DATA(y1 + 1);
    LCD_WR_DATA(y2 + 1);
    LCD_WR_REG(0x2c);
  }
  else if (USE_HORIZONTAL == 1)
  {
    LCD_WR_REG(0x2a);
    LCD_WR_DATA(x1 + 26);
    LCD_WR_DATA(x2 + 26);
    LCD_WR_REG(0x2b);
    LCD_WR_DATA(y1 + 1);
    LCD_WR_DATA(y2 + 1);
    LCD_WR_REG(0x2c);
  }
  else if (USE_HORIZONTAL == 2)
  {
    LCD_WR_REG(0x2a);
    LCD_WR_DATA(x1 + 1);
    LCD_WR_DATA(x2 + 1);
    LCD_WR_REG(0x2b);
    LCD_WR_DATA(y1 + 26);
    LCD_WR_DATA(y2 + 26);
    LCD_WR_REG(0x2c);
  }
  else
  {
    LCD_WR_REG(0x2a);
    LCD_WR_DATA(x1 + 1);
    LCD_WR_DATA(x2 + 1);
    LCD_WR_REG(0x2b);
    LCD_WR_DATA(y1 + 26);
    LCD_WR_DATA(y2 + 26);
    LCD_WR_REG(0x2c);
  }
}

#if SPI0_CFG == 2
void dma_config(void)
{
  dma_parameter_struct dma_init_struct;
  dma_deinit(DMA0, DMA_CH2);
  dma_struct_para_init(&dma_init_struct);

  dma_init_struct.periph_addr = (uint32_t)&SPI_DATA(SPI0);
  dma_init_struct.memory_addr = (uint32_t)image;
  dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;
  dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
  dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
  dma_init_struct.priority = DMA_PRIORITY_LOW;
  dma_init_struct.number = FRAME_SIZE;
  dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
  dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
  dma_init(DMA0, DMA_CH2, &dma_init_struct);
  dma_circulation_disable(DMA0, DMA_CH2);
  dma_memory_to_memory_disable(DMA0, DMA_CH2);
}
#endif

#if SPI0_CFG == 1
static void spi_config(void)
{
  spi_parameter_struct spi_init_struct;
  OLED_CS_Set();
  spi_struct_para_init(&spi_init_struct);

  spi_init_struct.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
  spi_init_struct.device_mode = SPI_MASTER;
  spi_init_struct.frame_size = SPI_FRAMESIZE_8BIT;
  spi_init_struct.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;
  spi_init_struct.nss = SPI_NSS_SOFT;
  spi_init_struct.prescale = SPI_PSC_8;
  spi_init_struct.endian = SPI_ENDIAN_MSB;
  spi_init(SPI0, &spi_init_struct);

  spi_crc_polynomial_set(SPI0, 7);
  spi_enable(SPI0);
}
#endif

void LCD_Init(void)
{
  rcu_periph_clock_enable(RCU_GPIOA);
  rcu_periph_clock_enable(RCU_GPIOB);

#if SPI0_CFG == 1
  rcu_periph_clock_enable(RCU_AF);
  rcu_periph_clock_enable(RCU_SPI0);

  gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
  gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2);

  spi_config();

#elif SPI0_CFG == 2
  rcu_periph_clock_enable(RCU_DMA0);
  rcu_periph_clock_enable(RCU_SPI0);

  gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_7);
  gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_6);

  dma_config();
  dma_channel_enable(DMA0, DMA_CH2);

#elif SPI0_CFG == 3
  gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_5 | GPIO_PIN_7);
  gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2);

  gpio_bit_reset(GPIOA, GPIO_PIN_5 | GPIO_PIN_7);
  gpio_bit_reset(GPIOB, GPIO_PIN_2);

#endif

  gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0 | GPIO_PIN_1);
  gpio_bit_reset(GPIOB, GPIO_PIN_0 | GPIO_PIN_1);

  OLED_RST_Clr();
  delay_1ms(200);
  OLED_RST_Set();
  delay_1ms(20);
  OLED_BLK_Set();

  LCD_WR_REG(0x11); // turn off sleep mode
  delay_1ms(100);

  LCD_WR_REG(0x21); // display inversion mode

  LCD_WR_REG(0xB1);   // Set the frame frequency of the full colors normal mode
  LCD_WR_DATA8(0x05); // RTNA
  LCD_WR_DATA8(0x3A); // FPA
  LCD_WR_DATA8(0x3A); // BPA

  LCD_WR_REG(0xB2);   // Set the frame frequency of the idel mode
  LCD_WR_DATA8(0x05); // RTNA
  LCD_WR_DATA8(0x3A); // FPA
  LCD_WR_DATA8(0x3A); // BPA

  LCD_WR_REG(0xB3);   // Set the frame fequency of the Partial mode / full colors
  LCD_WR_DATA8(0x05); // RTNA
  LCD_WR_DATA8(0x3A); // FPA
  LCD_WR_DATA8(0x3A); // BPA
  LCD_WR_DATA8(0x05); // RTNA
  LCD_WR_DATA8(0x3A); // FPA
  LCD_WR_DATA8(0x3A); // BPA

  LCD_WR_REG(0xB4);
  LCD_WR_DATA8(0x03);

  LCD_WR_REG(0xC0);
  LCD_WR_DATA8(0x62);
  LCD_WR_DATA8(0x02);
  LCD_WR_DATA8(0x04);

  LCD_WR_REG(0xC1);
  LCD_WR_DATA8(0xC0);

  LCD_WR_REG(0xC2);
  LCD_WR_DATA8(0x0D);
  LCD_WR_DATA8(0x00);

  LCD_WR_REG(0xC3);
  LCD_WR_DATA8(0x8D);
  LCD_WR_DATA8(0x6A);

  LCD_WR_REG(0xC4);
  LCD_WR_DATA8(0x8D);
  LCD_WR_DATA8(0xEE);

  LCD_WR_REG(0xC5);
  LCD_WR_DATA8(0x0E);

  LCD_WR_REG(0xE0);
  LCD_WR_DATA8(0x10);
  LCD_WR_DATA8(0x0E);
  LCD_WR_DATA8(0x02);
  LCD_WR_DATA8(0x03);
  LCD_WR_DATA8(0x0E);
  LCD_WR_DATA8(0x07);
  LCD_WR_DATA8(0x02);
  LCD_WR_DATA8(0x07);
  LCD_WR_DATA8(0x0A);
  LCD_WR_DATA8(0x12);
  LCD_WR_DATA8(0x27);
  LCD_WR_DATA8(0x37);
  LCD_WR_DATA8(0x00);
  LCD_WR_DATA8(0x0D);
  LCD_WR_DATA8(0x0E);
  LCD_WR_DATA8(0x10);

  LCD_WR_REG(0xE1);
  LCD_WR_DATA8(0x10);
  LCD_WR_DATA8(0x0E);
  LCD_WR_DATA8(0x03);
  LCD_WR_DATA8(0x03);
  LCD_WR_DATA8(0x0F);
  LCD_WR_DATA8(0x06);
  LCD_WR_DATA8(0x02);
  LCD_WR_DATA8(0x08);
  LCD_WR_DATA8(0x0A);
  LCD_WR_DATA8(0x13);
  LCD_WR_DATA8(0x26);
  LCD_WR_DATA8(0x36);
  LCD_WR_DATA8(0x00);
  LCD_WR_DATA8(0x0D);
  LCD_WR_DATA8(0x0E);
  LCD_WR_DATA8(0x10);

  LCD_WR_REG(0x3A);   // define the format of RGB picture data
  LCD_WR_DATA8(0x05); // 16-bit / pixel

  LCD_WR_REG(0x36);
  if (USE_HORIZONTAL == 0)
    LCD_WR_DATA8(0x08);
  else if (USE_HORIZONTAL == 1)
    LCD_WR_DATA8(0xC8);
  else if (USE_HORIZONTAL == 2)
    LCD_WR_DATA8(0x78);
  else
    LCD_WR_DATA8(0xA8);

  LCD_WR_REG(0x29); // Didplay On
}

void LCD_Clear(u16 color)
{
  u16 i, j;
  LCD_Address_Set(0, 0, LCD_W - 1, LCD_H - 1);
  for (i = 0; i < LCD_W; i++)
  {
    for (j = 0; j < LCD_H; j++)
    {
      LCD_WR_DATA(color);
    }
  }
}

void LCD_DrawPoint(u16 x, u16 y, u16 color)
{
  LCD_Address_Set(x, y, x, y);
  LCD_WR_DATA(color);
}

void LCD_DrawPoint_big(u16 x, u16 y, u16 color)
{
  LCD_Fill(x - 1, y - 1, x + 1, y + 1, color);
}

void LCD_Fill(u16 xsta, u16 ysta, u16 xend, u16 yend, u16 color)
{
  u16 i, j;
  LCD_Address_Set(xsta, ysta, xend, yend);
  for (i = ysta; i <= yend; i++)
  {
    for (j = xsta; j <= xend; j++)
    {
      LCD_WR_DATA(color);
    }
  }
}

void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2, u16 color)
{
  u16 t;
  int xerr = 0, yerr = 0, delta_x, delta_y, distance;
  int incx, incy, uRow, uCol;
  delta_x = x2 - x1;
  delta_y = y2 - y1;
  uRow = x1;
  uCol = y1;

  if (delta_x > 0)
    incx = 1;
  else if (delta_x == 0)
    incx = 0;
  else
  {
    incx = -1;
    delta_x = -delta_x;
  }

  if (delta_y > 0)
    incy = 1;
  else if (delta_y == 0)
    incy = 0;
  else
  {
    incy = -1;
    delta_y = -delta_y;
  }

  if (delta_x > delta_y)
    distance = delta_x;
  else
    distance = delta_y;
  for (t = 0; t < distance + 1; t++)
  {
    LCD_DrawPoint(uRow, uCol, color);
    xerr += delta_x;
    yerr += delta_y;
    if (xerr > distance)
    {
      xerr -= distance;
      uRow += incx;
    }
    if (yerr > distance)
    {
      yerr -= distance;
      uCol += incy;
    }
  }
}

void LCD_DrawRactangle(u16 x1, u16 y1, u16 x2, u16 y2, u16 color)
{
  LCD_DrawLine(x1, y1, x2, y1, color);
  LCD_DrawLine(x1, y1, x1, y2, color);
  LCD_DrawLine(x1, y2, x2, y2, color);
  LCD_DrawLine(x2, y1, x2, y2, color);
}

void LCD_DrawCircle(u16 x0, u16 y0, u8 r, u16 color)
{
  int a, b;
  a = 0;
  b = r;
  while (a <= b)
  {
    LCD_DrawPoint(x0 - b, y0 - a, color);
    LCD_DrawPoint(x0 + b, y0 - a, color);
    LCD_DrawPoint(x0 - a, y0 + b, color);
    LCD_DrawPoint(x0 - a, y0 - b, color);
    LCD_DrawPoint(x0 + b, y0 + a, color);
    LCD_DrawPoint(x0 + a, y0 - b, color);
    LCD_DrawPoint(x0 + a, y0 + b, color);
    LCD_DrawPoint(x0 - b, y0 + a, color);
    a++;
    if ((a * a + b * b) > (r * r))
    {
      b--;
    }
  }
}

void LCD_ShowChar(u16 x, u16 y, u8 num, u8 mode, u16 color)
{
  u8 tmp;
  u8 pos, t;
  u16 x0 = x;
  if (x > LCD_W - 16 || y > LCD_H - 16)
    return;
  num = num - ' ';
  LCD_Address_Set(x, y, x + 8 - 1, y + 16 - 1);
  if (!mode)
  {
    for (pos = 0; pos < 16; pos++)
    {
      tmp = asc2_1608[(u16)num * 16 + pos];
      for (t = 0; t < 8; t++)
      {
        if (tmp & 0x01)
          LCD_WR_DATA(color);
        else
          LCD_WR_DATA(BACK_COLOR);
        tmp >>= 1;
        x++;
      }
      x = x0;
      y++;
    }
  }
  else
  {
    for (pos = 0; pos < 16; pos++)
    {
      tmp = asc2_1608[(u16)num * 16 + pos];
      for (t = 0; t < 8; t++)
      {
        if (tmp & 0x01)
          LCD_DrawPoint(x + t, y + pos, color);
        tmp >>= 1;
      }
    }
  }
}

void LCD_ShowString(u16 x, u16 y, const u8 *p, u16 color)
{
  while (*p != '\0')
  {
    if (x > LCD_W - 16)
    {
      x = 0;
      y += 16;
    }
    if (y > LCD_H - 16)
    {
      x = y = 0;
      LCD_Clear(RED);
    }
    LCD_ShowChar(x, y, *p, 0, color);
    x += 8;
    p++;
  }
}

void LCD_ShowPicture(u16 x1, u16 y1, u16 x2, u16 y2)
{
  int i;
  LCD_Address_Set(x1, y1, x2, y2);
  for (i = 0; i < 12800; i++)
  {
    LCD_WR_DATA8(image[i]);
  }
}

void LCD_ShowLogo(void)
{
  u16 i, j;
  LCD_Address_Set(0, 0, LCD_W - 1, LCD_H - 1);
  for (i = 0; i < LCD_H; i++)
  {
    for (j = 0; j < LCD_W; j++)
    {
      LCD_WR_DATA(logo_rgb565[i * LCD_W + j]);
    }
  }
}