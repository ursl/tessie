//=================================================================================================
//
// Company:       Paul Scherrer Institut
//                5232 Villigen PSI
//                Switzerland
//
//-------------------------------------------------------------------------------------------------
//
// Project:       Peltier Controller V2
// Author:        Noah Piqué (noah.pique@psi.ch)
//
//-------------------------------------------------------------------------------------------------
//
// Module:        Error Handler
// Filename:      ERRH_ErrorHandler.h
// Date:          Handled by Subversion (version control system)
// Revision:      Handled by Subversion (version control system)
// History:       Handled by Subversion (version control system)
//
//-------------------------------------------------------------------------------------------------

#ifndef ERRH_ERRORHANDLER_H
#define ERRH_ERRORHANDLER_H

#ifdef __cplusplus
extern "C" {
#endif



//=================================================================================================
// Section:       INCLUDES
// Description:   List of required include files (visible by all modules).
//=================================================================================================

  
//=================================================================================================
// Section:       DEFINITIONS
// Description:   Definition of global constants (visible by all modules).
//=================================================================================================

// Error Definition

#define NO_ERROR 0x00000000

// CAN Errors

#define CAN_ERROR_MASK 0x01000000

// #define HAL_CAN_ERROR_NONE            (0x00000000U)  /*!< No error                                             */
// #define HAL_CAN_ERROR_EWG             (0x00000001U)  /*!< Protocol Error Warning                               */
// #define HAL_CAN_ERROR_EPV             (0x00000002U)  /*!< Error Passive                                        */
// #define HAL_CAN_ERROR_BOF             (0x00000004U)  /*!< Bus-off error                                        */
// #define HAL_CAN_ERROR_STF             (0x00000008U)  /*!< Stuff error                                          */
// #define HAL_CAN_ERROR_FOR             (0x00000010U)  /*!< Form error                                           */
// #define HAL_CAN_ERROR_ACK             (0x00000020U)  /*!< Acknowledgment error                                 */
// #define HAL_CAN_ERROR_BR              (0x00000040U)  /*!< Bit recessive error                                  */
// #define HAL_CAN_ERROR_BD              (0x00000080U)  /*!< Bit dominant error                                   */
// #define HAL_CAN_ERROR_CRC             (0x00000100U)  /*!< CRC error                                            */
// #define HAL_CAN_ERROR_RX_FOV0         (0x00000200U)  /*!< Rx FIFO0 overrun error                               */
// #define HAL_CAN_ERROR_RX_FOV1         (0x00000400U)  /*!< Rx FIFO1 overrun error                               */
// #define HAL_CAN_ERROR_TX_ALST0        (0x00000800U)  /*!< TxMailbox 0 transmit failure due to arbitration lost */
// #define HAL_CAN_ERROR_TX_TERR0        (0x00001000U)  /*!< TxMailbox 0 transmit failure due to transmit error   */
// #define HAL_CAN_ERROR_TX_ALST1        (0x00002000U)  /*!< TxMailbox 1 transmit failure due to arbitration lost */
// #define HAL_CAN_ERROR_TX_TERR1        (0x00004000U)  /*!< TxMailbox 1 transmit failure due to transmit error   */
// #define HAL_CAN_ERROR_TX_ALST2        (0x00008000U)  /*!< TxMailbox 2 transmit failure due to arbitration lost */
// #define HAL_CAN_ERROR_TX_TERR2        (0x00010000U)  /*!< TxMailbox 2 transmit failure due to transmit error   */
// #define HAL_CAN_ERROR_TIMEOUT         (0x00020000U)  /*!< Timeout error                                        */
// #define HAL_CAN_ERROR_NOT_INITIALIZED (0x00040000U)  /*!< Peripheral not initialized                           */
// #define HAL_CAN_ERROR_NOT_READY       (0x00080000U)  /*!< Peripheral not ready                                 */
// #define HAL_CAN_ERROR_NOT_STARTED     (0x00100000U)  /*!< Peripheral not started                               */
// #define HAL_CAN_ERROR_PARAM           (0x00200000U)  /*!< Parameter error                                      */
// #define HAL_CAN_ERROR_INTERNAL        (0x00800000U)  /*!< Internal error                                       */


// SPI Errors

#define SPI_ERROR_MASK 0x02000000

// #define HAL_SPI_ERROR_NONE              (0x00000000U)   /*!< No error                               */
// #define HAL_SPI_ERROR_MODF              (0x00000001U)   /*!< MODF error                             */
// #define HAL_SPI_ERROR_CRC               (0x00000002U)   /*!< CRC error                              */
// #define HAL_SPI_ERROR_OVR               (0x00000004U)   /*!< OVR error                              */
// #define HAL_SPI_ERROR_FRE               (0x00000008U)   /*!< FRE error                              */
// #define HAL_SPI_ERROR_DMA               (0x00000010U)   /*!< DMA transfer error                     */
// #define HAL_SPI_ERROR_FLAG              (0x00000020U)   /*!< Error on RXNE/TXE/BSY/FTLVL/FRLVL Flag */
// #define HAL_SPI_ERROR_ABORT             (0x00000040U)   /*!< Error during SPI Abort procedure       */

// Analog In Errors

#define AI_ERROR_MASK 0x03000000

// #define HAL_ADC_ERROR_NONE              (0x00U)   /*!< No error                                    */
// #define HAL_ADC_ERROR_INTERNAL          (0x01U)   /*!< ADC peripheral internal error (problem of clocking,
//                                                        enable/disable, erroneous state, ...)       */
// #define HAL_ADC_ERROR_OVR               (0x02U)   /*!< Overrun error                               */
// #define HAL_ADC_ERROR_DMA               (0x04U)   /*!< DMA transfer error                          */
// #define HAL_ADC_ERROR_JQOVF             (0x08U)   /*!< Injected context queue overflow error       */

#define AI_DMA_ERROR_MASK 0x04000000

// #define HAL_DMA_ERROR_NONE                 0x00000000U    /*!< No error                                */
// #define HAL_DMA_ERROR_TE                   0x00000001U    /*!< Transfer error                          */
// #define HAL_DMA_ERROR_NO_XFER              0x00000004U    /*!< Abort requested with no Xfer ongoing    */
// #define HAL_DMA_ERROR_TIMEOUT              0x00000020U    /*!< Timeout error                           */
// #define HAL_DMA_ERROR_NOT_SUPPORTED        0x00000100U    /*!< Not supported mode                      */
// #define HAL_DMA_ERROR_SYNC                 0x00000200U    /*!< DMAMUX sync overrun  error              */
// #define HAL_DMA_ERROR_REQGEN               0x00000400U    /*!< DMAMUX request generator overrun  error */

// Temperature Errors

#define TEMP_ERROR_MASK 0x05000000

#define TEMP_ERROR_SENSORM_MASK     0x00100000
#define TEMP_ERROR_SENSORW_MASK     0x00200000

#define TEMP_ERROR_SPI_FAILURE      0x00010000
#define TEMP_ERROR_GENERAL_FAILURE  0x00020000
#define TEMP_ERROR_SENSOR_FAILURE   0x00040000

// #define ADCD_FAULT_HIGHTHRESH 			0x80
// #define ADCD_FAULT_LOWTHRESH 			0x40
// #define ADCD_FAULT_REFINLOW 			    0x20
// #define ADCD_FAULT_REFINHIGH 			0x10
// #define ADCD_FAULT_RTDINLOW 			    0x08
// #define ADCD_FAULT_OVUV 				    0x04

// Variable Handler Errors

#define VARH_ERROR_MASK 0x06000000

#define VARH_ERROR_INVALID_VARIABLE 0x00000001      
#define VARH_ERROR_INVALID_DATA     0x00000002
#define VARH_ERROR_READONLY         0x00000004
#define VARH_ERROR_OUTOFRANGE       0x00000008
#define VARH_ERROR_OUTOFRANGE_INT   0x00000010
#define VARH_ERROR_LOAD_FLASH       0x00000020      // Error while loading from flash -> sets all variables to default


// Peltier Controller Errors

#define PELTIER_ERROR_MASK 0x07000000

#define PELTIER_ERROR_SETVOLTAGE        0x00000001
#define PELTIER_ERROR_SETVOLTAGE_PID    0x00000002
#define PELTIER_ERROR_MAX_POWER         0x00000004


// Main Application Errors

#define MAIN_ERROR_MASK 0x08000000

#define MAIN_ERROR_REG_NOT_FOUND            0x00000001
#define MAIN_ERROR_CMD_NOT_FOUND            0x00000002
#define MAIN_ERROR_SAVE_FLASH               0x00000004
#define MAIN_ERROR_WATCHDOG                 0x00000008  // not implemented

// HardFault Errors

#define HARDFAULT_ERROR_MASK 0x09000000

#define HARDFAULT_ERROR_IWDG    0x00000001
#define HARDFAULT_ERROR_SWRST   0x00000002  // Software Reset (if any hardfault occurs or user requests it via command)


//=================================================================================================
// Section:       MACROS
// Description:   Definition of global macros (visible by all modules).
//=================================================================================================



//=================================================================================================
// Section:       ENUMERATIONS
// Description:   Definition of global enumerations (visible by all modules).
//=================================================================================================





//=================================================================================================
// Section:       STRUCTURES
// Description:   Definition of global Structures (visible by all modules).
//=================================================================================================



//=================================================================================================
// Section:       GLOBAL VARIABLES
// Description:   Definition of global variables (visible by all modules).
//=================================================================================================



//=================================================================================================
// Section:       GLOBAL CONSTANTS
// Description:   Definition of global constants (visible by all modules).
//=================================================================================================



//=================================================================================================
// Section:       GLOBAL FUNCTIONS (PROTOTYPES)
// Description:   Definition of global functions (visible by all modules).
//=================================================================================================

#ifdef __cplusplus
}
#endif

#endif
