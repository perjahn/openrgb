/*-----------------------------------------*\
|  ThermaltakeRiingController.cpp           |
|                                           |
|  Definitions and types for Thermaltake    |
|  Riing Plus lighting controller           |
|                                           |
|  Adam Honse (CalcProgrammer1) 2/7/2020    |
\*-----------------------------------------*/

#include "ThermaltakeRiingController.h"

#include <cstring>

ThermaltakeRiingController::ThermaltakeRiingController(hid_device* dev_handle, const char* path)
{
    dev         = dev_handle;
    location    = path;

    SendInit();
}

ThermaltakeRiingController::~ThermaltakeRiingController()
{
    hid_close(dev);
}

std::string ThermaltakeRiingController::GetDeviceLocation()
{
    return("HID: " + location);
}

std::string ThermaltakeRiingController::GetSerialString()
{
    wchar_t serial_string[128];
    int ret = hid_get_serial_number_string(dev, serial_string, 128);

    if(ret != 0)
    {
        return("");
    }

    std::wstring return_wstring = serial_string;
    std::string return_string(return_wstring.begin(), return_wstring.end());

    return(return_string);
}

void ThermaltakeRiingController::GetFanData
    (
        unsigned char       port,
        unsigned char *     speed,
        unsigned short *    rpm
    )
{
    unsigned char usb_buf[64];

    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, sizeof(usb_buf));

    /*-----------------------------------------------------*\
    | Set up Get Fan Data packet                            |
    \*-----------------------------------------------------*/
    usb_buf[0x00]   = 0x33;
    usb_buf[0x01]   = 0x51;
    usb_buf[0x02]   = port;

    /*-----------------------------------------------------*\
    | Send packet                                           |
    \*-----------------------------------------------------*/
    hid_write(dev, usb_buf, 64);
    hid_read(dev, usb_buf, 64);

    *speed = usb_buf[0x04];
    *rpm   = (usb_buf[0x06] << 8) + usb_buf[0x05];
}

std::string ThermaltakeRiingController::GetFirmwareVersion()
{
    unsigned char usb_buf[64];

    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, sizeof(usb_buf));

    /*-----------------------------------------------------*\
    | Set up Get Firmware Version packet                    |
    \*-----------------------------------------------------*/
    usb_buf[0x00]   = 0x33;
    usb_buf[0x01]   = 0x50;

    /*-----------------------------------------------------*\
    | Send packet                                           |
    \*-----------------------------------------------------*/
    hid_write(dev, usb_buf, 64);
    hid_read(dev, usb_buf, 64);

    std::string ret_str = std::to_string(usb_buf[2]) + "." + std::to_string(usb_buf[3]) + "." + std::to_string(usb_buf[4]);

    return(ret_str);
}

void ThermaltakeRiingController::SetChannelLEDs(unsigned char channel, RGBColor * colors, unsigned int num_colors)
{
    unsigned char* color_data = new unsigned char[3 * num_colors];

    for(std::size_t color = 0; color < num_colors; color++)
    {
        int color_idx = color * 3;
        color_data[color_idx + 0] = RGBGetGValue(colors[color]);
        color_data[color_idx + 1] = RGBGetRValue(colors[color]);
        color_data[color_idx + 2] = RGBGetBValue(colors[color]);
    }

    SendRGB(channel + 1, current_mode, current_speed, num_colors, color_data);

    delete[] color_data;
}

void ThermaltakeRiingController::SetMode(unsigned char mode, unsigned char speed)
{
    current_mode        = mode;
    current_speed       = speed;
}

/*-------------------------------------------------------------------------------------------------*\
| Private packet sending functions.                                                                 |
\*-------------------------------------------------------------------------------------------------*/

void ThermaltakeRiingController::SendInit()
{
    unsigned char usb_buf[65];

    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, sizeof(usb_buf));

    /*-----------------------------------------------------*\
    | Set up Init packet                                    |
    \*-----------------------------------------------------*/
    usb_buf[0x00]   = 0x00;
    usb_buf[0x01]   = 0xFE;
    usb_buf[0x02]   = 0x33;

    /*-----------------------------------------------------*\
    | Send packet                                           |
    \*-----------------------------------------------------*/
    hid_write(dev, usb_buf, 65);
    hid_read(dev, usb_buf, 65);
}

void ThermaltakeRiingController::SendFan
    (
        unsigned char       port,
        unsigned char       mode,
        unsigned char       speed
    )
{
    unsigned char usb_buf[64];

    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, sizeof(usb_buf));

    /*-----------------------------------------------------*\
    | Set up RGB packet                                     |
    \*-----------------------------------------------------*/
    usb_buf[0x00]   = 0x32;
    usb_buf[0x01]   = 0x51;
    usb_buf[0x02]   = port;
    usb_buf[0x03]   = mode;
    usb_buf[0x04]   = speed;

    /*-----------------------------------------------------*\
    | Send packet                                           |
    \*-----------------------------------------------------*/
    hid_write(dev, usb_buf, 64);
    hid_read(dev, usb_buf, 64);
}

void ThermaltakeRiingController::SendRGB
    (
        unsigned char       port,
        unsigned char       mode,
        unsigned char       speed,
        unsigned char       num_colors,
        unsigned char*      color_data
    )
{
    unsigned char usb_buf[65];

    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, sizeof(usb_buf));

    /*-----------------------------------------------------*\
    | Set up RGB packet                                     |
    \*-----------------------------------------------------*/
    usb_buf[0x00]   = 0x00;
    usb_buf[0x01]   = 0x32;
    usb_buf[0x02]   = 0x52;
    usb_buf[0x03]   = port;
    usb_buf[0x04]   = mode + ( speed & 0x03 );

    /*-----------------------------------------------------*\
    | Copy in GRB color data                                |
    \*-----------------------------------------------------*/
    memcpy(&usb_buf[0x05], color_data, (num_colors * 3));

    /*-----------------------------------------------------*\
    | Send packet                                           |
    \*-----------------------------------------------------*/
    hid_write(dev, usb_buf, 65);
    hid_read(dev, usb_buf, 65);
}
