//
//  color.h
//  language
//
//  Created by Daniel Rehman on 1902192.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef color_h
#define color_h


#define use_colored_output 1


#if use_colored_output
#define cRED "\x1B[31m"
#define cGREEN "\x1B[32m"
#define cYELLOW "\x1B[33m"
#define cBLUE "\x1B[34m"
#define cMAGENTA "\x1B[35m"
#define cCYAN "\x1B[36m"
#define cWHITE "\x1B[37m"
#define cBRIGHT_GREEN "\x1B[92m"
#define cBRIGHT_RED "\x1B[091m"
#define cBRIGHT_BLUE "\x1B[94m"
#define cBRIGHT_YELLOW "\x1B[93m"
#define cBRIGHT_MAGENTA "\x1B[95m"
#define cBRIGHT_CYAN "\x1B[96m"
#define cGRAY "\x1B[90m"
#define cLIGHTGRAY "\x1B[37m"
#define cRESET "\x1B[0m"
#define cBOLDBLACK   "\033[1m\033[30m"
#define cBOLDRED     "\033[1m\033[31m"
#define cBOLDGREEN   "\033[1m\033[32m"
#define cBOLDYELLOW  "\033[1m\033[33m"
#define cBOLDBLUE    "\033[1m\033[34m"
#define cBOLDMAGENTA "\033[1m\033[35m"
#define cBOLDCYAN    "\033[1m\033[36m"
#define cBOLDWHITE   "\033[1m\033[37m"
#define cBOLDLIGHTGRAY "\033[1m\x1B[37m"
#define cBOLD "\033[1m"
#else
#define cRED ""
#define cGREEN ""
#define cYELLOW ""
#define cBLUE ""
#define cMAGENTA ""
#define cCYAN ""
#define cWHITE ""
#define cBRIGHT_GREEN ""
#define cBRIGHT_RED ""
#define cBRIGHT_BLUE ""
#define cBRIGHT_YELLOW ""
#define cBRIGHT_MAGENTA ""
#define cBRIGHT_CYAN ""
#define cGRAY ""
#define cLIGHTGRAY ""
#define cRESET ""
#define cBOLDBLACK   ""
#define cBOLDRED     ""
#define cBOLDGREEN   ""
#define cBOLDYELLOW  ""
#define cBOLDBLUE    ""
#define cBOLDMAGENTA ""
#define cBOLDCYAN    ""
#define cBOLDWHITE   ""
#define cBOLDLIGHTGRAY ""
#define cBOLD ""
#endif

#endif /* color_h */
