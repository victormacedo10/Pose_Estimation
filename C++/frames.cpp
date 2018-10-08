#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

using namespace std;
using namespace cv;
using namespace cv::dnn;

double j=0;

void* playback(void* dummy_ptr)
{
	// Imprime '1' continuamente em stderr.
	while(1)
	{
		j+=0.05;
		usleep(50000); // Atraso de 50000 us, ou 50 ms
	}
	return NULL;
}

int main ()
{
	pthread_t thread_id1;

	pthread_create (&thread_id1, NULL, &playback, NULL);

	while(1)
	{
		cout << j << endl;
		usleep(1000000);
	}
	return 0;
}