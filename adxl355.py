#Import necessary libraries
import spidev
# SPI configuration
SPI_MAX_CLOCK_HZ = 10000000
SPI_MIN_CLOCK_HZ = 100000
SPI_MODE = 0b00 # CPOL = 0, CPHA = 0
SPI_BUS = 0
SPI_DEVICE = 0
# ADXL355 addresses
DEVID_AD = 0x00
DEVID_MST = 0x01
PARTID = 0x02
REVID = 0x03
STATUS = 0x04
FIFO_ENTRIES = 0x05
TEMP2 = 0x06
TEMP1 = 0x07
XDATA3 = 0x08
XDATA2 = 0x09
XDATA1 = 0x0A
YDATA3 = 0x0B
YDATA2 = 0x0C
YDATA1 = 0x0D
ZDATA3 = 0x0E
ZDATA2 = 0x0F
ZDATA1 = 0x10
FIFO_DATA = 0x11
OFFSET_X_H = 0x1E
OFFSET_X_L = 0x1F
OFFSET_Y_H = 0x20
OFFSET_Y_L = 0x21
OFFSET_Z_H = 0x22
OFFSET_Z_L = 0x23
ACT_EN = 0x24
ACT_THRESH_H = 0x25
ACT_THRESH_L = 0x26
ACT_COUNT = 0x27
FILTER = 0x28
FIFO_SAMPLES = 0x29
INT_MAP = 0x2A
SYNC = 0x2B
RANGE = 0x2C
POWER_CTL = 0x2D
SELF_TEST = 0x2E
RESET = 0x2F
# Data Range
RANGE_2G = 0x01
RANGE_4G = 0x02
RANGE_8g = 0x03
# Values
WRITE_BIT = 0x00
READ_BIT = 0x01
DUMMY_BYTE = 0xAA
MEASURE_MODE = 0x06

class ADXL355:
    def twos_comp(val, bits):
        """compute the 2's complement of int value val"""
        if (val & (1 << (bits - 1))) != 0: # if sign bit is set e.g., 8bit: 128-255
            val = val - (1 << bits)        # compute negative value
        return val                         # return positive value as is
    """
    Class to interact with ADXL355 device
    Allows user to read, write and obtain data
    from the accelerometer
    """
    def __init__(self, measure_range=RANGE_2G):
        """Initializes sensor and SPI bus.
        Args:
        measure_range (int): measure range selected
        Returns:
        None
        """
        # Initialize SPI
        self.spi = spidev.SpiDev()
        self.spi.open(SPI_BUS, SPI_DEVICE)
        self.spi.max_speed_hz = SPI_MAX_CLOCK_HZ
        self.spi.mode = SPI_MODE
        # Initialize sensor
        self._set_measure_range(measure_range)
        self._enable_measure_mode()
    def write_data(self, address, value):
        """Writes data on ADXL355 device address.
        Args:
        address (int): Address to write in ADXL355.
        value (int): Value to write in address.
        Returns:
        None
        """
        device_address = address << 1 | WRITE_BIT
        self.spi.xfer2([device_address, value])
    def read_data(self, address):
        """Reads data from ADXL355 device.
        Args:
        address (int): Address to read from ADXL355.
        Returns:
        int: Value in speficied address in accelerometer
        """
        device_address = address << 1 | READ_BIT
        return self.spi.xfer2([device_address, DUMMY_BYTE])[1]
    def _set_measure_range(self, measure_range):
        """Sets measure range on ADXL355 device.
        Args:
        measure_range (int): Measure range to set in ADXL355.
        Returns:
        None
        """
        # Write data
        self.write_data(RANGE, measure_range)
    def get_measure_range(self):
        """Gets measure range
        Returns:
        measure_range (int): Measure range
        """
        # Read data
        raw_data = self.read_data(RANGE)
        # Split data
        measure_range = (raw_data - ((raw_data >> 2) << 2))
        # Return values
        return measure_range
    def _enable_measure_mode(self):
        """
        Enables measure mode on ADXL355 device.
        Returns:
        None
        """
        # Write data
        self.write_data(POWER_CTL, MEASURE_MODE)
    def set_measure_mode(self, drdy_off, temp_off, standby):
        """Sets measure mode
        Returns:
        None
        """
        # Read register before modifying it
        data = self.read_data(POWER_CTL)
        # Preserve reserved data, discard the rest
        data = ((data >> 3) << 3)
        # Add measure mode
        data = data + (drdy_off << 2) + (temp_off << 1) + standby
        # Write data
        self.write_data(POWER_CTL, data)
    def get_axes(self):
        """
        Gets the current data from the axes.
        Returns:
        dict: Current value for x, y and z axis
        """
        # Reading data
        _data = [self.read_data(XDATA1), self.read_data(XDATA2), \
        self.read_data(XDATA3)]
        y_data = [self.read_data(YDATA1), self.read_data(YDATA2), \
        self.read_data(YDATA3)]
        z_data = [self.read_data(ZDATA1), self.read_data(ZDATA2), \
        self.read_data(ZDATA3)]
        # Join data
        x_data = (x_data[0] >> 4) + (x_data[1] << 4) \
        + (x_data[2] << 12)
        y_data = (y_data[0] >> 4) + (y_data[1] << 4) \
        + (y_data[2] << 12)
        z_data = (z_data[0] >> 4) + (z_data[1] << 4) \
        + (z_data[2] << 12)
        # Apply two complement
        x_data = twos_comp(x_data, 20)
        y_data = twos_comp(y_data, 20)
        z_data = twos_comp(z_data, 20)
        # Return values
        return [x_data, y_data, z_data]
    def get_temperature(self):
        """Get current temperature.
        Returns:
        (int): Current temperature in 12bit
        """
        # Reading data
        temp_data = [self.read_data(TEMP1), self.read_data(TEMP2)]
        # Join data
        temp_data = (temp_data[0]) + ((temp_data[1] \
        - ((temp_data[1] >> 4) << 4)) << 8)
        # Return values
        return temp_data
    def get_axes_and_temp(self):
        """Get current accelerations and temperature
        Returns:
        (dict): x, y, z, temperature
        """
        # Reading data
        x_data = [self.read_data(XDATA1), self.read_data(XDATA2), \
        self.read_data(XDATA3)]
        y_data = [self.read_data(YDATA1), self.read_data(YDATA2), \
        self.read_data(YDATA3)]
        z_data = [self.read_data(ZDATA1), self.read_data(ZDATA2), \
        self.read_data(ZDATA3)]
        temp_data = [self.read_data(TEMP1), self.read_data(TEMP2)]
        # Join data
        x_data = (x_data[0] >> 4) + (x_data[1] << 4) \
        + (x_data[2] << 12)
        y_data = (y_data[0] >> 4) + (y_data[1] << 4) \
        + (y_data[2] << 12)
        z_data = (z_data[0] >> 4) + (z_data[1] << 4) \
        + (z_data[2] << 12)
        temp_data = ((temp_data[1] - ((temp_data[1] >> 4) << 4)) \
        << 8) + (temp_data[0] << 0)
        # Apply two complement
        if (x_data & (1 << 19)) != 0:
            x_data = x_data - (1 << 20)
        if (y_data & (1 << 19)) != 0:
            y_data = y_data - (1 << 20)
        if (z_data & (1 << 19)) != 0:
            z_data = z_data - (1 << 20)
        # Return values
        return [-x_data, -y_data, -z_data, temp_data]
    def set_ODR_and_filter(self, odr_lpf, hpf_filter):
        """Set Low Pass Filter, Output Data Rate (ODR) and
        High Pass Filter (HPF)
        Returns:
        None
        """
        # Read register before modifying
        data = self.read_data(FILTER)
        # Preserve reserved data
        data = ((data >> 7) << 7)
        # Join data
        data = data + (hpf_filter << 4) + odr_lpf
        # Write data
        self.write_data(FILTER, data)
    def get_ODR_and_filter(self):
        """Gets Low Pass Filter, Output Data Rate (ODR) and
        High Pass Filter (HPF)
        Returns:
        (int): odr_lpf (ODR and LPF bits),
        hpf_filter (HPF bits)
        """
        # Read data
        raw_data = self.read_data(FILTER)
        odr_lpf = raw_data - ((raw_data >> 4) << 4)
        hpf_filter = (raw_data >> 4)
        hpf_filter = hpf_filter - ((hpf_filter >> 3) << 3)
        # Return values
        return [odr_lpf, hpf_filter]
    def set_sync(self, ext_clk, ext_sync):
        """Sets type of synchronization
        Returns:
        None
        """
        # Read data on register before modifiying
        data = self.read_data(SYNC)
        # Preserve reserved bits
        data = ((data >> 3) << 3)
        # Join data, not modifying reserved data
        data = (data) + (ext_clk << 2) + ext_sync
        # Write data
        self.write_data(SYNC, data)
    def get_sync(self):
        """Gets type of synchronization
        Returns:
        (int): synchronization type
        """
        # Read data
        raw_data = self.read_data(SYNC)
        # Split bits
        ext_sync = raw_data - ((raw_data >> 2) << 2)
        ext_clk = (raw_data >> 2)
        ext_clk = ext_clk - ((ext_clk >> 1) << 1)
        # Return values
        return [ext_clk, ext_sync]
    def get_status(self):
        """Get status
        Returns:
        (int):
        """
        # Read data
        raw_data = self.read_data(STATUS)
        # Split bits
        data_rdy = raw_data - ((raw_data >> 1) << 1)
        raw_data = (raw_data >> 1)
        fifo_full = raw_data - ((raw_data >> 1) << 1)
        raw_data = (raw_data >> 1)
        fifo_ovr = raw_data - ((raw_data >> 1) << 1)
        raw_data = (raw_data >> 1)
        activity = raw_data - ((raw_data >> 1) << 1)
        raw_data = (raw_data >> 1)
        nvm_busy = raw_data - ((raw_data >> 1) << 1)
        # Return values
        return [data_rdy, fifo_full, fifo_ovr, activity, \
        nvm_busy]
    def get_offsets(self):
        """ Get X, Y, Z offsets
        Return:
        ():
        """
        # Read data
        x_data = [self.read_data(OFFSET_X_H), \
        self.read_data(OFFSET_X_L)]
        y_data = [self.read_data(OFFSET_Y_H), \
        self.read_data(OFFSET_Y_L)]
        z_data = [self.read_data(OFFSET_Z_H), \
        self.read_data(OFFSET_Z_L)]
        # Join data
        x_data = x_data[1] + (x_data[0] << 8)
        y_data = y_data[1] + (y_data[0] << 8)
        z_data = z_data[1] + (z_data[0] << 8)
        # The significance of OFFSET[15:0] matches
        # the significance of DATA[19:4]
        x_data = (x_data << 4)
        y_data = (y_data << 4)
        z_data = (z_data << 4)
        # Apply tow complement
        x_data = twos_comp(x_data, 20)
        y_data = twos_comp(y_data, 20)
        z_data = twos_comp(z_data, 20)
        # Return values
        return [x_data, y_data, z_data]
    def reset_settings(self):
        """Reset all settings
        Returns:
        None
        """
        # Reset command
        self.write_data(RESET, 0x52)

