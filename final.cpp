#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <iostream> 
// Physical base address of GPIO
const unsigned gpio_address = 0x400d0000;
// Length of memory-mapped IO window
const unsigned gpio_size = 0xff;
const int gpio_led1_offset = 0x12C; // Offset for LED1
const int gpio_led2_offset = 0x130; // Offset for LED2
const int gpio_led3_offset = 0x134; // Offset for LED3
const int gpio_led4_offset = 0x138; // Offset for LED4
const int gpio_led5_offset = 0x13C; // Offset for LED5
const int gpio_led6_offset = 0x140; // Offset for LED6
const int gpio_led7_offset = 0x144; // Offset for LED7
const int gpio_led8_offset = 0x148; // Offset for LED8
const int gpio_sw1_offset = 0x14C; // Offset for Switch 1
const int gpio_sw2_offset = 0x150; // Offset for Switch 2
const int gpio_sw3_offset = 0x154; // Offset for Switch 3
const int gpio_sw4_offset = 0x158; // Offset for Switch 4
const int gpio_sw5_offset = 0x15C; // Offset for Switch 5
const int gpio_sw6_offset = 0x160; // Offset for Switch 6
const int gpio_sw7_offset = 0x164; // Offset for Switch 7
const int gpio_sw8_offset = 0x168; // Offset for Switch 8
const int gpio_pbtnl_offset = 0x16C; // Offset for left push button
const int gpio_pbtnr_offset = 0x170; // Offset for right push button
const int gpio_pbtnu_offset = 0x174; // Offset for up push button
const int gpio_pbtnd_offset = 0x178; // Offset for down push button
const int gpio_pbtnc_offset = 0x17C; // Offset for center push button 

/**
 * Write a 4-byte value at the specified general-purpose I/O location.
 *
 * @param ptr Base address returned by 'mmap'.
 * @parem offset Offset where device is mapped.
 * @param value Value to be written.
 */
void RegisterWrite(char *ptr, int offset, int value)
{
	* (int *) (ptr + offset) = value;
}

/**
 * Read a 4-byte value from the specified general-purpose I/O location.
 *
 * @param ptr Base address returned by 'mmap'.
 * @param offset Offset where device is mapped.
 * @return Value read.
 */
int RegisterRead(char *ptr, int offset)
{
	return * (int *) (ptr + offset);
}

/**
 * Initialize general-purpose I/O
 * - Opens access to physical memory /dev/mem
 * - Maps memory at offset 'gpio_address' into virtual address space
 *
 * @param fd File descriptor passed by reference, where the result
 * of function 'open' will be stored.
 * @return Address to virtual memory which is mapped to physical,
 * or MAP_FAILED on error.
 */
char *Initialize(int *fd)
{
	*fd = open( "/dev/mem", O_RDWR);
	return (char *) mmap(
	NULL,
	gpio_size,
	PROT_READ | PROT_WRITE,
	MAP_SHARED,
	*fd,
	gpio_address);
}

/**
 * Close general-purpose I/O.
 *
 * @param ptr Virtual address where I/O was mapped.
 * @param fd File descriptor previously returned by 'open'.
 */
void Finalize(char *ptr, int fd)
{
	munmap(ptr, gpio_size);
	close(fd);
} 

/**
 * Converts signed decimal to signed binary on zedboard. produces and error if overflow occurs  
 *
 * @param ptr Base address of I/O
 * @param value Value to show on LEDs
 */
void SetLedNumber(char *ptr, int value)
{
	if((value > 127) || (value < -128)) // Overflow 8-bit signed numbers -2^(n-1) to 2^(n-1) - 1
	{
		std::cout<< "OVERFLOW HAS OCCURED, ERROR in result shown" << std::endl;
		value = 0; 
  	}
	
  	if( value < 0 )
  	{
  		value = value + 256;
  	}

  	RegisterWrite(ptr, gpio_led1_offset, value % 2);
  	RegisterWrite(ptr, gpio_led2_offset, (value / 2) % 2);
  	RegisterWrite(ptr, gpio_led3_offset, (value / 4) % 2);
  	RegisterWrite(ptr, gpio_led4_offset, (value / 8) % 2);
  	RegisterWrite(ptr, gpio_led5_offset, (value / 16) % 2);
  	RegisterWrite(ptr, gpio_led6_offset, (value / 32) % 2);
  	RegisterWrite(ptr, gpio_led7_offset, (value / 64) % 2);
 	RegisterWrite(ptr, gpio_led8_offset, (value / 128) % 2);
}

/** Set the state of the LED with the given index.
 *
 * @param ptr Base address for general-purpose I/O
 * @parem led_index LED index between 0 and 7
 * @param state Turn on (1) or off (0)
 */
void SetLedState(char *ptr, int led_index, int state)
{
  	int led; 

	switch (led_index) {
  		case 0:
    		led = gpio_led1_offset;
    		break;
  		case 1:
    		led = gpio_led2_offset;
    		break;
  		case 2:
    		led = gpio_led3_offset;
    		break;
  		case 3:
    		led = gpio_led4_offset;
    		break;
  		case 4:
    		led = gpio_led5_offset;
    		break;
  		case 5:
    		led = gpio_led6_offset;
    		break;
  		case 6:
    		led = gpio_led7_offset;
    		break;
  		case 7:
    		led = gpio_led8_offset;
    		break;
	}

  RegisterWrite(ptr, led, state);
}

// takes the value from all the given leds and adds them together
// in order to return a decimal integer.
int BinaryToDecimal(char *ptr)
{
  	int value = ((RegisterRead(ptr, gpio_sw1_offset) * 1) +
  		(RegisterRead(ptr, gpio_sw2_offset) * 2) +
		(RegisterRead(ptr, gpio_sw3_offset) * 4) +
		(RegisterRead(ptr, gpio_sw4_offset) * 8) +
		(RegisterRead(ptr, gpio_sw5_offset) * 16) +
		(RegisterRead(ptr, gpio_sw6_offset) * 32) +
		(RegisterRead(ptr, gpio_sw7_offset) * 64) +
		(RegisterRead(ptr, gpio_sw8_offset) * -128));
	return value;
}

int main()
{
	// Initialize
  	int fd;
  	char *ptr = Initialize(&fd);
  	// Check error
    if (ptr == MAP_FAILED)
    {
		perror("Mapping I/O memory failed - Did you run with 'sudo'?\n");
      	return -1;
    }

  	printf("enter while loop...\n");

    int op1 = 0; //the first operand
    int op2 = 0; //the second operand
    int operation = 2; //either 0 = +, 1 = - and 2 = unassigned 
    int up = 0; //adds the numbers together
    int right = 0; //displays the addition or subtraction of two numbers...equals sign
    int down = 0; //subtract the numbers together
    int left = 0;
    int center = 0;
	int value = 0;

  	while (1) 
  	{
 		SetLedState(ptr, 0, RegisterRead(ptr, gpio_sw1_offset));
    	SetLedState(ptr, 1, RegisterRead(ptr, gpio_sw2_offset));
    	SetLedState(ptr, 2, RegisterRead(ptr, gpio_sw3_offset));
    	SetLedState(ptr, 3, RegisterRead(ptr, gpio_sw4_offset));
    	SetLedState(ptr, 4, RegisterRead(ptr, gpio_sw5_offset));
    	SetLedState(ptr, 5, RegisterRead(ptr, gpio_sw6_offset));
    	SetLedState(ptr, 6, RegisterRead(ptr, gpio_sw7_offset));
    	SetLedState(ptr, 7, RegisterRead(ptr, gpio_sw8_offset));

    	int _up = RegisterRead(ptr, gpio_pbtnu_offset);
    	int _right = RegisterRead(ptr, gpio_pbtnr_offset);
    	int _down = RegisterRead(ptr, gpio_pbtnd_offset);
    	int _left = RegisterRead(ptr, gpio_pbtnl_offset);
    	int _center = RegisterRead(ptr, gpio_pbtnc_offset);
  
        //////INPUT FOR OPERATION AND FIRST OPERAND /////////////
		// store op1 and assigns the operation to addition 
    	if (up != _up) 
    	{  
      		up = _up;
      		if (_up == 1)
      		{
        		op1 = BinaryToDecimal(ptr);
				operation = 0; //0 is addition 
				std::cout << op1 << std::endl; 
				std::cout << "operation " << operation << std::endl; 
      		}
    	}

		// store op1 and assigns the operation to subtraction  
    	if (down != _down) 
    	{
      		down = _down;
      		if (_down == 1) 
      		{
        		op1 = BinaryToDecimal(ptr);
				operation = 1; // 1 is subtraction 
				std::cout <<"op1 " << op1 << std::endl; 
				std::cout << "operation " << operation << std::endl; 
      		}
    	}
        
        //////INPUT FOR EQUALS COMMAND AND SECOND OPERAND /////////////
    	if (right != _right) 
	    {
	      	right = _right;

	      	if (_right == 1) 
	      	{
	        	op2 = BinaryToDecimal(ptr);
				std::cout << op2 << std::endl;
	        	if (operation == 0) //Addition
	        	{
	          		value = op1 + op2;
	          		printf("op1 + op2 = " );
	        	}
	        	else if (operation == 1) // Subtraction
	        	{
	          		value = op1 - op2;
	          		printf("op1 - op2 = ");
	        	}
	        	else if (operation = 2) //Error
	        	{
					std::cout << "ERROR: equals command requested but no operation was selected. Please try again!" << std::endl;
				}
                
        //////OUTPUTING RESULTS AND RESETING /////////////
                
	      		SetLedNumber(ptr, value); // Set LEDs to signed output
				std::cout << value << std::endl;  // Output the value on the terminal
				sleep(5); // Give the user a chance to see output
		  		SetLedNumber(ptr, -1); // flash red so the user knows Zed has been reset
				sleep(1); 
				//reset variables to starting values so another input can occur
				op1 = 0;
				op2 = 0;
				operation = 2;
	    	}
    	}
	}

  	printf("End loop.\n");
  	// Done
  	Finalize(ptr, fd);
  	return 0;
}