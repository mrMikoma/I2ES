#include <avr/io.h>

#ifndef PINS_H
#define PINS_H

#define EMERGENCY_INT_DDR       DDRD
#define EMERGENCY_INT_PORT      PORTD 
#define EMERGENCY_INT_PIN       PD3

#define LCD_PORT                PORTA           /**< port for the LCD lines   */
#define LCD_DATA0_PORT          PORTE           /**< port for 4bit data bit 0 */
#define LCD_DATA1_PORT          PORTG           /**< port for 4bit data bit 1 */
#define LCD_DATA2_PORT          PORTE           /**< port for 4bit data bit 2 */
#define LCD_DATA3_PORT          PORTH           /**< port for 4bit data bit 3 */
#define LCD_DATA0_PIN           5               /**< pin for 4bit data bit 0  */
#define LCD_DATA1_PIN           5               /**< pin for 4bit data bit 1  */
#define LCD_DATA2_PIN           3               /**< pin for 4bit data bit 2  */
#define LCD_DATA3_PIN           3               /**< pin for 4bit data bit 3  */
#define LCD_RS_PORT             PORTH           /**< port for RS line         */
#define LCD_RS_PIN              6               /**< pin  for RS line         */
#define LCD_RW_PORT             PORTB           /**< port for RW line         */
#define LCD_RW_PIN              4               /**< pin  for RW line         */
#define LCD_E_PORT              PORTB           /**< port for Enable line     */
#define LCD_E_PIN               5               /**< pin  for Enable line     */

#define M_RowColDirection       DDRK            //PORT Direction Configuration for keypad
#define M_ROW                   PORTK           //Higher four bits of PORT are used as ROWs
#define M_COL                   PINK            //Lower four bits of PORT are used as COLs
#define C_RowOutputColInput_U8  0xf0	        //value to configure Rows as Output and Columns as Input

#endif

