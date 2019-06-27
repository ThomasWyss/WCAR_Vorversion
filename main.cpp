//#include <cstdio>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>			//Used for UART
#include <termios.h>		//Used for UART

#include <linux/i2c-dev.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//#include <wiringPiI2C.h>


constexpr auto AnzahlByteRead = 64;

void scan_i2c_bus(int device)
{
	int port, res;

	for (port = 0; port < 127; port++)
	{
		if (ioctl(device, I2C_SLAVE, port) < 0) 
		{
			printf("\nPort 0x%x", port);
			perror("ioctl() I2C_SLAVE failed");
		}
	
		else
		{
			res = i2c_smbus_read_byte(device);
			if (res >= 0) printf("i2c chip found at: 0x%x, val = %d\n", port, res);
		}
	}
}

int rs232_init()
{
	//-------------------------
	//----- SETUP USART 0 -----
	//-------------------------
	//At bootup, pins 8 and 10 are already set to UART0_TXD, UART0_RXD (ie the alt0 function) respectively
	int uart0_filestream = -1;

	//OPEN THE UART
	//The flags (defined in fcntl.h):
	//	Access modes (use 1 of these):
	//		O_RDONLY - Open for reading only.
	//		O_RDWR - Open for reading and writing.
	//		O_WRONLY - Open for writing only.
	//
	//	O_NDELAY / O_NONBLOCK (same function) - Enables nonblocking mode. When set read requests on the file can return immediately with a failure status
	//											if there is no input immediately available (instead of blocking). Likewise, write requests can also return
	//											immediately with a failure status if the output can't be written immediately.
	//
	//	O_NOCTTY - When set and path identifies a terminal device, open() shall not cause the terminal device to become the controlling terminal for the process.
	//uart0_filestream = open("/dev/serial0", O_RDWR | O_NOCTTY | O_NDELAY);		//Open in non blocking read/write mode
	uart0_filestream = open("/dev/ttyAMA0", O_RDWR | O_NOCTTY | O_NDELAY);		//Open in non blocking read/write mode
	if (uart0_filestream == -1)
	{
		//ERROR - CAN'T OPEN SERIAL PORT
		printf("Error - Unable to open UART.  Ensure it is not in use by another application\n");
	}

	//CONFIGURE THE UART
	//The flags (defined in /usr/include/termios.h - see http://pubs.opengroup.org/onlinepubs/007908799/xsh/termios.h.html):
	//	Baud rate:- B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B500000, B576000, B921600, B1000000, B1152000, B1500000, B2000000, B2500000, B3000000, B3500000, B4000000
	//	CSIZE:- CS5, CS6, CS7, CS8
	//	CLOCAL - Ignore modem status lines
	//	CREAD - Enable receiver
	//	IGNPAR = Ignore characters with parity errors
	//	ICRNL - Map CR to NL on input (Use for ASCII comms where you want to auto correct end of line characters - don't use for bianry comms!)
	//	PARENB - Parity enable
	//	PARODD - Odd parity (else even)
	struct termios options;
	tcgetattr(uart0_filestream, &options);
	options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;		//<Set baud rate
	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;
	tcflush(uart0_filestream, TCIFLUSH);
	tcsetattr(uart0_filestream, TCSANOW, &options);

	return uart0_filestream;
}





int main()
{
	unsigned long funcs;
	int iFile, abc;
	int iByte;
	int res[AnzahlByteRead];

	printf("WCAR_Vorversion V0.0\n");


	int rs232device=rs232_init();


	//----- TX BYTES -----
	unsigned char tx_buffer[20];
	unsigned char *p_tx_buffer;

	p_tx_buffer = &tx_buffer[0];
	*p_tx_buffer++ = 'H';
	*p_tx_buffer++ = 'e';
	*p_tx_buffer++ = 'l';
	*p_tx_buffer++ = 'l';
	*p_tx_buffer++ = 'o';

	if (rs232device != -1)
	{
		int count = write(rs232device, &tx_buffer[0], (p_tx_buffer - &tx_buffer[0]));		//Filestream, bytes to write, number of bytes to write
		if (count < 0)
		{
			printf("UART TX error\n");
		}
	}

	//----- CHECK FOR ANY RX BYTES -----
	if (rs232device != -1)
	{
		// Read up to 255 characters from the port if they are there
		unsigned char rx_buffer[256];
		int rx_length = read(rs232device, (void*)rx_buffer, 255);		//Filestream, buffer to store in, number of bytes to read (max)
		if (rx_length < 0)
		{
			printf("UART RX error <0\n");
		}
		else if (rx_length == 0)
		{
			printf("UART RX error ==0\n");
		}
		else
		{
			//Bytes received
			rx_buffer[rx_length] = '\0';
			printf("%i bytes read : %s\n", rx_length, rx_buffer);
		}
	}




	if (( iFile = open("/dev/i2c-1", O_RDWR))<0)
	{
		perror("Failed to open the i2c bus");
		exit(1);
	}
	if ((abc = ioctl(iFile, I2C_SLAVE, 0x28)) < 0)
	{
		perror("Failed to acquire bus access and/or talk to slave.\n");
		exit(1);
	}
	else
	{
		printf("i2c return = %d ioctl = %d\n", iFile, abc);
	}


	/* Abfragen, ob die I2C-Funktionen da sind */
	if (ioctl(iFile, I2C_FUNCS, &funcs) < 0)
	{
		perror("ioctl() I2C_FUNCS failed");
		exit(1);
	}

	
	/* Ergebnis untersuchen */
	if (funcs & I2C_FUNC_I2C)				printf("I2C\n");
	if (funcs & (I2C_FUNC_SMBUS_BYTE))		printf("I2C_FUNC_SMBUS_BYTE\n");
	if (funcs & (I2C_FUNC_SMBUS_READ_BYTE))	printf("I2C_FUNC_SMBUS_READ_BYTE\n");	
	if (funcs & I2C_FUNC_SMBUS_QUICK)		printf("I2C_FUNC_SMBUS_QUICK\n\n");

	scan_i2c_bus(iFile);
	ioctl(iFile, I2C_SLAVE, 0x28);
	for (iByte = 0; iByte < AnzahlByteRead; iByte++)
	{
		res[iByte] = i2c_smbus_read_byte(iFile);
	}

	printf("\n*** END ***");
	return 0;
}