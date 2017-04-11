//
// Copyright (c) 2014 Mikhail Korytov
// All Rights Reserved
//

#include <QApplication>

#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <ifaddrs.h>
#include <netdb.h>

#include <iostream>
#include <fstream>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>

#include "lvtcpserver.h"
#include "maintcpserver.h"

#include "bcontrol.h"
#include "bcamera.h"

bControl *bc = NULL;
bCamera *cam;

void exitGraceful(int sig) {
    qDebug() << "extitting";
   
    delete cam;

    qDebug() << "...done";

    exit(0);
}

/*
std::string determineAddress() {
    ifaddrs *ifaddr, *ifa;
    int n;
    getifaddrs(&ifaddr);
//    printf("Browsing over ifs\r\n");
    for (ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++) {
//        printf("Trying next if\r\n");
        if (ifa->ifa_addr == NULL || ifa->ifa_addr->sa_family != AF_INET)continue;
        if(0==strcmp(ifa->ifa_name, "eth0")){
//            printf("Got eth0...");
            
            char host[NI_MAXHOST];
            int s = getnameinfo(ifa->ifa_addr,
                                sizeof(struct sockaddr_in),
                                host, NI_MAXHOST,
                                NULL, 0, NI_NUMERICHOST);
//            printf("address: <%s>\n", host);
            freeifaddrs(ifaddr);
            return std::string(host);
        }
    }
    //TODO return something!
    return std::string("");
}
*/

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

/*
    cam = new bCamera(1, 12, 1, 14, 1, 16);
    qDebug() << "initCam";

    while(1);
*/
    qDebug() << "Starting up";

    qDebug() << argc << " " << argv[0] << " "  << argv[1] << " ...done";

    bool dontDaemonize = false;
    if (argc > 1 && std::string(argv[1]) == std::string("n")) {
        qDebug() << "non-daemon";
        dontDaemonize = true;
    }

    if (dontDaemonize == false) {
        qDebug() << "daemonizing";
        int pid, sid;
        pid = fork();

        if (pid < 0) {
            qDebug() << "Can't fork";
            exit(1);
        } else if (pid != 0) {
            exit(0);
        }

        sid = setsid();

        if (sid < 0) {
            qDebug() << "Can't setsid";
            exit(1);
        }
        umask(0);

        chdir("/");
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
    }
    
    std::ofstream out("/emmc/dkv2/bone.log", std::ofstream::app);
    std::streambuf *coutbuf = std::cout.rdbuf();
    std::cout.rdbuf(out.rdbuf());

    std::ofstream outErr("/emmc/dkv2/bone-err.log", std::ofstream::app);
    std::streambuf *cerrbuf = std::cerr.rdbuf();
    std::cerr.rdbuf(outErr.rdbuf());

    signal(SIGPIPE, SIG_IGN);
    
    // To enable correct finishing of the program using signals
    signal(SIGINT, &exitGraceful);
    signal(SIGTERM, &exitGraceful);

/*
    tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
    printf("INFO: Starting %s example.\r\n", "blinkslave");
    prussdrv_init ();
    
    unsigned int ret;
    ret = prussdrv_open(PRU_EVTOUT_0);
    if (ret)
    {
        printf("prussdrv_open open failed\n");
        return (ret);
    }
    prussdrv_pruintc_init(&pruss_intc_initdata);
    printf("INFO: Initializing example.\r\n");
    
    prussdrv_map_prumem (PRUSS0_PRU0_DATARAM, &pruDataMem);
    pruDataMem_byte = (unsigned char*) pruDataMem;
    
    printf("INFO: Executing example.\r\n");
    prussdrv_exec_program (PRU_NUM, "/home/korytov/bone/pru/blinkslave.bin");
    
    printf("INFO: PRU ready\r\n");
*/
    bc = new bControl();

    cam = new bCamera(148, 158, 160); //AF: P9_16, SR: P9_14

    QThread *thread_cam = new QThread();
    cam->moveToThread(thread_cam);
    thread_cam->start();

    bc->setCamera(cam);

//    std::string currentAddress = determineAddress();
    
//    std::cout << "current address: " << currentAddress.c_str() << "\r\n";

    MainTcpServer mainsrv;
    mainsrv.setControl(bc);

    LvTcpServer lvsrv;
    lvsrv.setCamera(cam);

    return a.exec();
}
