#include <cstdio>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>

int main()
{
	unsigned long funcs;
	int iFile, abc;

	printf("WCAR_Vorversion V0.0\n");

	if (( iFile = open("/dev/i2c-1", O_RDWR))<0)
	{
		perror("Failed to open the i2c bus");
		exit(1);
	}
	if ((abc = ioctl(iFile, I2C_SLAVE, 0x20)) < 0)
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

	///* Ergebnis untersuchen */
	//if (funcs & I2C_FUNC_I2C)
	//	printf("I2C\n");
	//if (funcs & (I2C_FUNC_SMBUS_BYTE))
	//	printf("I2C_FUNC_SMBUS_BYTE\n");
	
	return 0;
}