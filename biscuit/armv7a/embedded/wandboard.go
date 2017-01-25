package embedded

//SPI1 Pins
// MOSI EIM D18

var WB_JP4_4 = GPIO_pin{"JP4_4", 3, 11, gpios[3-1], IOMUX_MUX_CTL_GPIO3, IOMUX_PAD_CTL_GPIO3}
var WB_JP4_6 = GPIO_pin{"JP4_6", 3, 27, gpios[3-1], IOMUX_MUX_CTL_GPIO3, IOMUX_PAD_CTL_GPIO3}
var WB_JP4_8 = GPIO_pin{"JP4_8", 6, 31, gpios[6-1], IOMUX_MUX_CTL_GPIO6, IOMUX_PAD_CTL_GPIO6}
var WB_JP4_10 = GPIO_pin{"JP4_10", 1, 24, gpios[1-1], IOMUX_MUX_CTL_GPIO1, IOMUX_PAD_CTL_GPIO1}

var WB_SPI1 = SPI_periph{SPI_pin{"mosi", 1, IOMUX_MUX_CTL_EIM_D17, IOMUX_PAD_CTL_EIM_D17},
	SPI_pin{"miso", 1, IOMUX_MUX_CTL_EIM_D18, IOMUX_PAD_CTL_EIM_D18},
	SPI_pin{"sclk", 1, IOMUX_MUX_CTL_EIM_D16, IOMUX_PAD_CTL_EIM_D16},
	SPI_pin{"chip select", 0, IOMUX_MUX_CTL_KEY_COL2, IOMUX_PAD_CTL_KEY_COL2},
	0,
	0,
	1}
