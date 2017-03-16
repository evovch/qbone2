//
// Simple LED control example
//

#include <stdio.h>
#include <iostream>

#include "bonelib/gpio.hpp"
#include "bonelib/kbdio.hpp"
#include <unistd.h>
#include <time.h>

int
main(int argc, const char *argv[])
{
    std::cout << "Hello GPIO!\n";

 
    
    
    BeagleBone::gpio* CS2 = BeagleBone::gpio::P8(11);
    CS2->configure(BeagleBone::pin::OUT);
    CS2->set(0);
    while (1) {
        time_t now = time(0);
        tm* localtm = localtime(&now);
        
        for(int i=0; i < localtm->tm_hour; i++){
            CS2->set(1);
            usleep( 0.1 * 1000 * 1000 );
            CS2->set(0);
            usleep( 0.1 * 1000 * 1000 );
        }
        usleep( 5 * 1000 * 1000 );
    }
//    CS->set(0);
    return 1;
}
