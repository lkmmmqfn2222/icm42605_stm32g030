
del /Q MDK-ARM\*.bak
del /Q MDK-ARM\*.dep
del /Q MDK-ARM\JLinkLog.txt
del /Q MDK-ARM\*.uvgui.*
del /Q MDK-ARM\*.uvguix.*
rd /Q /S MDK-ARM\stm32g030f6

del /Q EWARM\*.bak
del /Q EWARM\*.dep
del /Q EWARM\JLinkLog.txt
rd /Q /S EWARM\stm32g030f6

rd /Q /S Output\IAR
rd /Q /S Output\MDK
exit



