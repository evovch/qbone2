QT       += core
QT       -= gui
CONFIG += console

release: DESTDIR = ../dkvlogo-build-release
debug:   DESTDIR = ../dkvlogo-build-debug

OBJECTS_DIR = $$DESTDIR/.obj
MOC_DIR = $$DESTDIR/.moc
RCC_DIR = $$DESTDIR/.qrc
UI_DIR = $$DESTDIR/.ui

QMAKE_CXXFLAGS += -std=c++11 -fpermissive -pthread -I../qbone/u8glib/src/ -I/opt/arm/include -I/opt/arm/include/arm-linux-gnueabihf -I/home/korytov/libs/include
QMAKE_LFLAGS += -pthread -L/opt/arm/lib/arm-linux-gnueabihf -L/opt/arm/lib -Wl,-O1 -Wl,-rpath,/opt/arm/lib,-rpath,/opt/arm/arm-linux-gnueabihf/lib -lrt
QMAKE_CFLAGS += -Iu8glib/src/ -I/opt/arm/include -I/opt/arm/include/arm-linux-gnueabihf -I/home/korytov/libs/include

SOURCES += main.cpp \
    ../qbone/display.cpp \
    ../qbone/u8glib/beaglebone/common/u8g_arm.cpp \
    ../qbone/u8glib/src/chessengine.c \
    ../qbone/u8glib/src/u8g_bitmap.c \
    ../qbone/u8glib/src/u8g_circle.c \
    ../qbone/u8glib/src/u8g_clip.c \
    ../qbone/u8glib/src/u8g_com_api_16gr.c \
    ../qbone/u8glib/src/u8g_com_api.c \
    ../qbone/u8glib/src/u8g_com_i2c.c \
    ../qbone/u8glib/src/u8g_com_io.c \
    ../qbone/u8glib/src/u8g_com_null.c \
    ../qbone/u8glib/src/u8g_cursor.c \
    ../qbone/u8glib/src/u8g_delay.c \
    ../qbone/u8glib/src/u8g_dev_a2_micro_printer.c \
    ../qbone/u8glib/src/u8g_dev_flipdisc_2x7.c \
    ../qbone/u8glib/src/u8g_dev_gprof.c \
    ../qbone/u8glib/src/u8g_dev_ht1632.c \
    ../qbone/u8glib/src/u8g_dev_ili9325d_320x240.c \
    ../qbone/u8glib/src/u8g_dev_ks0108_128x64.c \
    ../qbone/u8glib/src/u8g_dev_lc7981_160x80.c \
    ../qbone/u8glib/src/u8g_dev_lc7981_240x64.c \
    ../qbone/u8glib/src/u8g_dev_lc7981_240x128.c \
    ../qbone/u8glib/src/u8g_dev_lc7981_320x64.c \
    ../qbone/u8glib/src/u8g_dev_ld7032_60x32.c \
    ../qbone/u8glib/src/u8g_dev_null.c \
    ../qbone/u8glib/src/u8g_dev_pcd8544_84x48.c \
    ../qbone/u8glib/src/u8g_dev_pcf8812_96x65.c \
    ../qbone/u8glib/src/u8g_dev_sbn1661_122x32.c \
    ../qbone/u8glib/src/u8g_dev_ssd1306_128x32.c \
    ../qbone/u8glib/src/u8g_dev_ssd1306_128x64.c \
    ../qbone/u8glib/src/u8g_dev_ssd1309_128x64.c \
    ../qbone/u8glib/src/u8g_dev_ssd1322_nhd31oled_bw.c \
    ../qbone/u8glib/src/u8g_dev_ssd1322_nhd31oled_gr.c \
    ../qbone/u8glib/src/u8g_dev_ssd1325_nhd27oled_bw_new.c \
    ../qbone/u8glib/src/u8g_dev_ssd1325_nhd27oled_bw.c \
    ../qbone/u8glib/src/u8g_dev_ssd1325_nhd27oled_gr_new.c \
    ../qbone/u8glib/src/u8g_dev_ssd1325_nhd27oled_gr.c \
    ../qbone/u8glib/src/u8g_dev_ssd1327_96x96_gr.c \
    ../qbone/u8glib/src/u8g_dev_ssd1351_128x128.c \
    ../qbone/u8glib/src/u8g_dev_st7565_64128n.c \
    ../qbone/u8glib/src/u8g_dev_st7565_dogm128.c \
    ../qbone/u8glib/src/u8g_dev_st7565_dogm132.c \
    ../qbone/u8glib/src/u8g_dev_st7565_lm6059.c \
    ../qbone/u8glib/src/u8g_dev_st7565_lm6063.c \
    ../qbone/u8glib/src/u8g_dev_st7565_nhd_c12832.c \
    ../qbone/u8glib/src/u8g_dev_st7565_nhd_c12864.c \
    ../qbone/u8glib/src/u8g_dev_st7687_c144mvgd.c \
    ../qbone/u8glib/src/u8g_dev_st7920_128x64.c \
    ../qbone/u8glib/src/u8g_dev_st7920_192x32.c \
    ../qbone/u8glib/src/u8g_dev_st7920_202x32.c \
    ../qbone/u8glib/src/u8g_dev_t6963_128x64.c \
    ../qbone/u8glib/src/u8g_dev_t6963_128x128.c \
    ../qbone/u8glib/src/u8g_dev_t6963_240x64.c \
    ../qbone/u8glib/src/u8g_dev_t6963_240x128.c \
    ../qbone/u8glib/src/u8g_dev_tls8204_84x48.c \
    ../qbone/u8glib/src/u8g_dev_uc1601_c128032.c \
    ../qbone/u8glib/src/u8g_dev_uc1608_240x64.c \
    ../qbone/u8glib/src/u8g_dev_uc1610_dogxl160.c \
    ../qbone/u8glib/src/u8g_dev_uc1701_dogs102.c \
    ../qbone/u8glib/src/u8g_dev_uc1701_mini12864.c \
    ../qbone/u8glib/src/u8g_ellipse.c \
    ../qbone/u8glib/src/u8g_font_data.c \
    ../qbone/u8glib/src/u8g_font.c \
    ../qbone/u8glib/src/u8g_line.c \
    ../qbone/u8glib/src/u8g_ll_api.c \
    ../qbone/u8glib/src/u8g_page.c \
    ../qbone/u8glib/src/u8g_pb.c \
    ../qbone/u8glib/src/u8g_pb8h1.c \
    ../qbone/u8glib/src/u8g_pb8h1f.c \
    ../qbone/u8glib/src/u8g_pb8h2.c \
    ../qbone/u8glib/src/u8g_pb8h8.c \
    ../qbone/u8glib/src/u8g_pb8v1.c \
    ../qbone/u8glib/src/u8g_pb8v2.c \
    ../qbone/u8glib/src/u8g_pb14v1.c \
    ../qbone/u8glib/src/u8g_pb16h1.c \
    ../qbone/u8glib/src/u8g_pb16h2.c \
    ../qbone/u8glib/src/u8g_pb16v1.c \
    ../qbone/u8glib/src/u8g_pb16v2.c \
    ../qbone/u8glib/src/u8g_pb32h1.c \
    ../qbone/u8glib/src/u8g_pbxh16.c \
    ../qbone/u8glib/src/u8g_pbxh24.c \
    ../qbone/u8glib/src/u8g_polygon.c \
    ../qbone/u8glib/src/u8g_rect.c \
    ../qbone/u8glib/src/u8g_rot.c \
    ../qbone/u8glib/src/u8g_scale.c \
    ../qbone/u8glib/src/u8g_state.c \
    ../qbone/u8glib/src/u8g_u8toa.c \
    ../qbone/u8glib/src/u8g_u16toa.c \
    ../qbone/u8glib/src/u8g_virtual_screen.c

HEADERS += \
    ../qbone/display.h \
    ../qbone/u8glib/beaglebone/common/u8g_arm.h \
    ../qbone/u8glib/src/u8g.h
