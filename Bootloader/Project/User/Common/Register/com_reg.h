#ifndef __COM_REG_H__
#define __COM_REG_H__




typedef struct {       

	
  volatile  uint32_t  DR;   /**<UART Data Register>*/                                                   
  volatile  uint32_t  RSR;  /**<UART Status Register>*/   
	volatile  uint32_t  FR;   /**<Flag Register>*/   
	volatile  uint32_t  ILPR; /**<IrDA Counter>*/  
	volatile  uint32_t  IBRD; /**<Integer Baud Rate Divisor>*/
	volatile  uint32_t  FBRD;	/**<Fractional Baud Rate Divisor>*/
	volatile  uint32_t  LCRH;	/**<Line Control High>*/
	volatile  uint32_t  CR;	  /**<Control Register>*/
	volatile  uint32_t  IFLS;	/**<FIFO Interrupt Level Select>*/
	volatile  uint32_t  IER;	/**<Interrupt Enable>*/
	volatile  uint32_t  IES;	/**<Interrupt Status>*/
	volatile  uint32_t  MIS;	/**<Masked Interrupt Status>*/
	volatile  uint32_t  IEC;	/**<Interrupt Clear>*/
                                        
} APOLLO2_UART1_Type;

#define APOLLO2_UART1_BASE                  0x4001D000UL
#define APOLLO2_UART1                       ((APOLLO2_UART1_Type           *) APOLLO2_UART1_BASE)



#endif //__COM_REG_H__
