# fcmd
fcmd is a simple c-function unit debug library.
use it uart or console to test your unit.

# how to use

# history:
0.03	add va_arg param support，eg:int printf(const char *fmt, ...)
0.02	add string param support, eg:disp(50,-60, "ADC:  mV", 10, "89"), 
	add mem cmd.
0.01	test ok，only support int32_t param.

# note
set _args_t wide，32-bit mcu use int，
but 16-bit and 8-bit mcu，int maybe 16-bit，8-bit mcu maybe use two reg to pass short param(high-byte&low-byte),
