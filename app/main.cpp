#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <sys/time.h>

#ifdef __linux__
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#elif defined(__uCnix__)
#include <armv8-m/sys/i2c.h>
#endif


//simple placeholder until i figure out my timer api in the uCnix kernel
//very ugly since i spam the kernel with syscalls which isnt that bad per ce since syscalls are handled in thread mode since a syscall just swaps the current thread with a per task kernel handler but its more efficient to block the task on a oneshot timer which i will do when i have rewritten the scheduler, locking mechanisms and other threading related code including the exec mechanics since everything is dogshit right now.

static void simple_delay(uint32_t ms) {
	
	struct timespec lastTime;
	clock_gettime(CLOCK_MONOTONIC, &lastTime);
	
	while (1) {
		struct timespec current;
		clock_gettime(CLOCK_MONOTONIC, &current);
		uint32_t elapsed = (current.tv_nsec - lastTime.tv_nsec)/1000000U + (current.tv_sec - lastTime.tv_sec)*1000U;

		if (elapsed > ms) {
			break;
		}
	}
}



class I2CBus {
	private:
		int bus_fd;
		uint8_t address;

	public:
		I2CBus() {
			this->bus_fd = -1;
			this->address = -1;
		}

		~I2CBus() {
			if (this->bus_fd != -1) {
				close(this->bus_fd);
			}
		}

		int bind(const char* device, uint8_t address) {
			if (this->bus_fd != -1) {
				close(this->bus_fd);
			}
			int fd = open(device, O_RDWR);
			if (fd < 0) return -1;
			this->bus_fd = fd;
			
			struct i2c_msg msg = {
				.addr = address,
				.flags = 0,
				.len = 0,
				.buf = NULL
			};

			struct i2c_rdwr_ioctl_data data = {
				.msgs = &msg,
				.nmsgs = 1
			};

			if (ioctl(this->bus_fd, I2C_RDWR, &data) != 0) return -1;

			this->address = address;
			return 0;
		}

		int write(int reg, const uint8_t* buffer, int count) {
			if (this->bus_fd == -1) return -1;

			uint8_t reg_buff = reg;
			struct i2c_msg msg[2] = { //START:ADDRESS:ACK:REG:ACK:DATA...
				{
					.addr = this->address,
					.flags = 0,
					.len = 1,
					.buf = &reg_buff
				},
				{
					.flags = I2C_M_NOSTART,
					.len = (uint8_t)count,
					.buf = (uint8_t*) buffer
				}
			};

			struct i2c_rdwr_ioctl_data data = {
				.msgs = msg,
				.nmsgs = 2
			};

			return ioctl(this->bus_fd, I2C_RDWR, &data);
		}

		int read(int reg, uint8_t* buffer, uint8_t count) {
			if (this->bus_fd == -1) return -1;

			uint8_t reg_buff = reg;
			struct i2c_msg msg[2] = {
				{ //send START:ADDRESS:ACK:REG:ACK:RESTART:ADDRESS:ACK:DATA0:ACK:DATA1...
					.addr = this->address,
					.flags = 0,
					.len = 1,
					.buf = &reg_buff
				},
				{
					.addr = this->address,
					.flags = I2C_M_RD,
					.len = count,
					.buf = buffer
				}
			};

			struct i2c_rdwr_ioctl_data data = {
				.msgs = msg,
				.nmsgs = 2
			};

			return ioctl(this->bus_fd, I2C_RDWR, &data);
		}
};


struct BMP280Calibration {
    uint16_t T1;
    int16_t  T2, T3;
    uint16_t P1;
    int16_t  P2, P3, P4, P5, P6, P7, P8, P9;
};



class BMP280Sensor {
	private:
		int32_t t_fine;
		uint8_t regId;
		BMP280Calibration calibrationData;

		I2CBus& bus;

	public:
		BMP280Sensor(I2CBus& bus) : bus(bus) {
			this->regId = 0;
		}

		int init() {
			if (this->bus.read(0xD0, &this->regId, 1) < 0) {
				puts("BMP280Sensor::init()\tCant read from device.\n");
				return -1;
			}

			if (this->regId != 0x58) {
				puts("BMP280Sensor::init()\tDevice id is incorrect.\n");
				return -1;
			}

			uint8_t regReset = 0xB6;
			this->bus.write(0xE0, &regReset, 1); //reset device
			
			uint8_t regStatus;
			int i;
			for (i = 0; i < 100; i++) {
				if (this->bus.read(0xF3, &regStatus, 1) < 0) {
					puts("BMP280Sensor::init()\tCant read status register.\n");
					return -1;
				}

				simple_delay(10);
				
				if (!(regStatus & (1 << 0))) { //check nvm copy finished using im_update flag
					break;
				}
			}
			if (i == 100) {
				puts("BMP280Sensor::init()\tInitialization time exceeded.\n");
				return -1;
			}

			if (this->bus.read(0x88, (uint8_t*) &this->calibrationData, sizeof(BMP280Calibration)) < 0) {
				puts("BMP280Sensor::init()\tCant read calibration data.\n");
				return -1;
			}

			uint8_t configReg = 0x20;
			if (this->bus.write(0xF5, &configReg, 1) < 0) {
				puts("BMP280Sensor::init()\tCant write configuration register.\n");
				return -1;
			}

			uint8_t ctrl_meas = 0x2F;
			if (bus.write(0xF4, &ctrl_meas, 1) < 0) {
				puts("BMP280Sensor::init()\tCant write ctrl_meas register.\n");
				return -1;
			}

			return 0;
		}

		int getTemperature() {
			uint8_t data[3];
			if (this->bus.read(0xFA, data, 3) < 0) {
				puts("BMP280Sensor::getTemperature()\tCant read temperature registers.\n");
				return 0;
			}

			int32_t adc_T = ((int32_t)data[0] << 12) | ((int32_t)data[1] << 4) | ((int32_t) data[2] >> 4);

			int32_t var1, var2, T, T1, T2, T3;
			T1 = this->calibrationData.T1;
			T2 = this->calibrationData.T2;
			T3 = this->calibrationData.T3;
			
			var1 = ((((adc_T>>3) - ((int32_t)T1<<1))) * ((int32_t)T2)) >> 11;
			var2 = (((((adc_T>>4) - ((int32_t)T1)) * ((adc_T>>4) - ((int32_t)T1))) >> 12) * ((int32_t)T3)) >> 14;
			this->t_fine = var1 + var2;
			T = (this->t_fine * 5 + 128) >> 8;
			return T;
		}
};

int main(int argc, char** argv)
{
	if (argc != 3) {
		puts("bmp280: expected: app [DEV] [ADDRESS]\n");
		return 1;
	}
	I2CBus bus;
	
	if (bus.bind(argv[1], atoi(argv[2])) == -1) {
		puts("bmp280: failed to bind i2c bus to target\n");
		return 1;
	}

	BMP280Sensor sensor(bus);
	
	if (sensor.init() < 0) {
		puts("bmp280: initialization failed\n");
		return 1;
	}

	while (1) {
		simple_delay(500); //wait 100 ms until the next measurement

		int reading = sensor.getTemperature();
		
		printf("bmp280: temperature=%d C\r", reading);
	}
	
	return 0;
}

