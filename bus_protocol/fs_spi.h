#ifndef FS_SPI_H
#define FS_SPI_H

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <string>
#include <stdint.h>
#include "ispi.h"
#include "../gpioint.h"
#include <QDebug>

//#define TI_MODE 1

/** @brief SPI implementation for Linux systems using file devices
 *
 *  This class implements an interface to the SPI buses provided via the /dev
 *  device file interface on Linux systems.  It implements the ISPI interface
 *  to allow SPI dependent devices to access these devices without being aware
 *  of the Linux file-system symantics.
 *
 */
class FS_SPI : public ISPI
{
protected:
    int 	    m_fd;
    std::string m_fname;
    uint16_t    m_spiDelay;             // Don't know what this is for

#ifdef TI_MODE
    gpioInt *csnPin1, *csnPin2, *csnPin3;
#endif

public:
    FS_SPI(const std::string& fn);
    FS_SPI(int speed, const std::string& fn);
    virtual ~FS_SPI();

    int openBus();
    int closeBus();
    int isReady();
    int setBPW(int val);
    int setSpeed(int val);
    int setMode(int val);
#ifdef TI_MODE
    int selectCSN(int n);
#endif
    int      rwData(uint8_t *data, uint8_t len);
    uint8_t  rwByte(uint8_t bt);
    uint16_t rwWord(uint16_t wd);

protected:
    void init(int speed, const std::string& fn);
};





/** @brief Construct an SPI object on the file device with a default clock rate.
 *
 *  @param fn std::string containing the filename to open.
 */
inline FS_SPI::FS_SPI(const std::string& fn)
{
    init(500000, fn);
}


// driver 1: 000
// driver 2: 100
// driver 3: 010
// driver 4: 110
// driver 5: 001

#ifdef TI_MODE
inline int FS_SPI::selectCSN(int n) {
    switch(n) {
    case 1:
         csnPin1->setValue(0);
         csnPin2->setValue(0);
         csnPin3->setValue(0);
         break;
    case 2:
        csnPin1->setValue(1);
        csnPin2->setValue(0);
        csnPin3->setValue(0);
        break;
    case 3:
        csnPin1->setValue(0);
        csnPin2->setValue(1);
        csnPin3->setValue(0);
        break;
    case 4:
        csnPin1->setValue(1);
        csnPin2->setValue(1);
        csnPin3->setValue(0);
        break;
    case 5:
        csnPin1->setValue(0);
        csnPin2->setValue(0);
        csnPin3->setValue(1);
        break;
    default:
        csnPin1->setValue(0);
        csnPin2->setValue(0);
        csnPin3->setValue(0);
        break;
    }
    return 1;
}
#endif

/** @brief Construct an SPI object on the file device with the requested clock rate.
 *
 *  @param speed Clock speed for the SPI bus.
 *  @param fn The std::string containing the filename to open.
 */
inline FS_SPI::FS_SPI(int speed, const std::string& fn)
{
    init(speed, fn);

#ifdef TI_MODE
    csnPin1 = new gpioInt(30);
    csnPin2 = new gpioInt(60);
    csnPin3 = new gpioInt(31);
    csnPin1->setDir(1);
    csnPin2->setDir(1);
    csnPin3->setDir(1);
#endif

}

/** @brief Indicates that the SPI object has a valid file handle.
  *
  * @return int: 1 - Has a valid file handle to the SPI device file. 0 - Otherwise.
  */
inline int FS_SPI::isReady()
{
    if(m_fd > 0)
        return 1;
    else
        return 0;
}


inline FS_SPI::~FS_SPI()
{}


/** @brief init - Initialize the object with starting values.
 *
 * Initialize the object with basic starting values for speed
 * and channel.
 *
 * @param speed Speed in Hz for the SPI clock.
 * @param ch The SPI CS line that is used for this object's communication.
 */
inline void FS_SPI::init(int speed, const std::string& fn)
{
    m_fname     = fn;
    m_speed     = speed;
    m_spiMode   = 0;
    m_spiBPW    = 8;
    m_spiDelay  = 0;
    m_fd        = 0;
}


/** @brief Opens the device file for the bus and sets the parameters.
 *
 * Sets up the bus to do read/write operations using the current settings for
 * this object.
 *
 */
inline int FS_SPI::openBus()
{
    if (m_fd > 0)
    {
        printf("SPI::openBus: bus already opened - [%08X]\n", m_fd);
        return 0;
    }
    
    int fd = open(m_fname.c_str(), O_RDWR) ;

    if (fd  < 0)
    {
        perror("SPI::openBus: ");
        return -1 ;
    }

    // Setup the SPI bus with our current parameters
    if (ioctl (fd, SPI_IOC_WR_MODE, &m_spiMode)         < 0)
    {
        perror("SPI::openBus: ");
        return -1 ;
    }
    
    if (ioctl (fd, SPI_IOC_RD_MODE, &m_spiMode)         < 0)
    {
        perror("SPI::openBus: ");
        return -1 ;
    }
    
    if (ioctl (fd, SPI_IOC_WR_BITS_PER_WORD, &m_spiBPW) < 0)
    {
        perror("SPI::openBus: ");
        return -1 ;
    }
    
    if (ioctl (fd, SPI_IOC_RD_BITS_PER_WORD, &m_spiBPW) < 0)
    {
        perror("SPI::openBus: ");
        return -1 ;
    }
    
    if (ioctl (fd, SPI_IOC_WR_MAX_SPEED_HZ, &m_speed)   < 0)
    {
        perror("SPI::openBus: ");
        return -1 ;
    }
    
    if (ioctl (fd, SPI_IOC_RD_MAX_SPEED_HZ, &m_speed)   < 0)
    {
        perror("SPI::openBus: ");
        return -1 ;
    }

    m_fd = fd;
    
    return 0;
}


/** @brief closeBus - Closes the device file and terminates the transfer of data.
 */
inline int FS_SPI::closeBus()
{
    if (m_fd > 0)
    {
        close(m_fd);
        m_fd = 0;
        return 0;
    }
    else
        
        return m_fd;
}


/** @brief setBPW - Sets the
 *
 * Sets the 'Bits per Word' parameter for subsequent SPI transfers.
 *
 * @param val The number of bits to transfer for each SPI 'word'
 */
inline int FS_SPI::setBPW(int val)
{
    int result = m_spiBPW;
    m_spiBPW = val;
    return result;
}


/** @brief setSpeed - Set the bus clock speed
 *
 * Sets the bus clock speed to the requested value.  Note that
 * only certain speeds are valid.  The speed is actually determined
 * by divding the core frequency by powers of two.  The actual clock
 * rate will be the closest value that does not exceed this parameter.
 * for a 250MHz core ex:
 *    (61,032 | 122,064 | 244,128 | 488,256 | 976,512 | 1,953,024 | 3,906,048...)
 *
 * @param val The requested clock frequency in Hz.
 */
inline int FS_SPI::setSpeed(int val)
{
    int result = m_speed;
    m_speed = val;
    return result;
}


/** @brief Sets the SPI operation mode.
 *
 *  Sets one of 4 modes allowed by SPI to determine the clock polarity
 *  and leading or trailing edge data bit latching.
 *  @param val The SPI mode to use
 */
inline int FS_SPI::setMode(int val)
{
    if (val<0)
        val = 0;
    if (val>3)
        val = 3;
    int result = m_spiMode;
    m_spiMode = val;
    return result;
}


/** @brief Shifts data out as well as in.
 *
 * Sends the data contained in the buffer to the bus and reads the incomming
 * data from the bus.  The buffer is overwritten with the incoming data.
 *
 * @param data Pointer to a buffer of data to send.
 * @param len Number of bytes to send from the buffer.
 */

/*
inline int FS_SPI::rwData(uint8_t* data, uint8_t len)
{
    uint8_t data_out = *data;
    uint8_t data_in;

    int ret;
 //   ret = write(m_fd, &data_out, len);
    qDebug() << "write: " << *data;
    if(ret < 0)return(ret);
    ret = read(m_fd, &data_in, len);

    *data = data_in;
    qDebug() << "read: " << ret << "bytes: " << data_in;
    return ret;
}
*/

inline int FS_SPI::rwData(uint8_t* data, uint8_t len)
{
    struct spi_ioc_transfer spiCtrl;

//    memset (&spiCtrl, 0, sizeof(spiCtrl));

//    qDebug() << "sent: " << *data;

//    data[0] = 'a';
      spiCtrl.tx_buf        =  (unsigned long)data,
      spiCtrl.rx_buf        =  (unsigned long)data,
    spiCtrl.len           = len;
    spiCtrl.delay_usecs   = 0;
    spiCtrl.speed_hz      = 500000;
    spiCtrl.bits_per_word = 8;
    spiCtrl.tx_nbits = 0;
    spiCtrl.rx_nbits = 0;
    spiCtrl.pad = 0;
    spiCtrl.cs_change = 0;


//    qDebug() << "received: " << *data;



    int ret;
    ret = ioctl(m_fd, SPI_IOC_MESSAGE(1), &spiCtrl);

    return(ret);
}


/** @brief Send and recieve one 8 bit byte of data.
 *
 * @param bt Data byte to send.
 */
inline uint8_t FS_SPI::rwByte(uint8_t bt)
{
    setBPW(8);
    rwData(&bt, 1);
    return bt;
}


/** @brief Send and recieve one 16 bit word of data.
 *
 * @param wd Data word to send.
 */
inline uint16_t FS_SPI::rwWord(uint16_t wd)
{
    setBPW(8);
    rwData((uint8_t*)&wd, 2);
    return wd;
}






/*
 Copyright (C) 2013 Kyle Crane
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal in
 the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#endif // FS_SPI_H
