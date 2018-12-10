#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <pthread.h>

using namespace std;

int main(int argc, char **argv)
{
    
    string port = "/dev/rfcomm0";
    int device = open(port.c_str(), O_RDWR | O_NOCTTY | O_SYNC);

    cout << "Bluetooth Connected" << endl;

    string response;
    char buffer[3];

    while(1)
    {
        int n = read(device, buffer, sizeof(buffer));

        if (n > 0) {
            response += string(buffer);
            cout << buffer << endl;
        }

    }

    cout << "Response is: " << endl;
    cout << response << endl;

    close(device);
    return 0;
}