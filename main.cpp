#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <sys/time.h>
#include <opencv2/opencv.hpp>
#include <signal.h>

#include "IPC_shm.hpp"

#define SHM_QUEUE_NAME 1

static void * cap;
static float fps = 0;
double demo_time;
static int demo_done = 0;
static double frame_time = 0;

sigset_t newset;
sigset_t zeroset;
static int sigFlag = 0;

const char shm_name[SHM_QUEUE_NAME][32] =
{
    "/dev/shm/imageshm001"/*,"/dev/shm/imageshm002",
    "/dev/shm/imageshm003","/dev/shm/imageshm004",
    "/dev/shm/imageshm005","/dev/shm/imageshm006",
    "/dev/shm/imageshm007","/dev/shm/imageshm008"*/
};

// The conversion here is very slow
/*typedef struct
{
    char year[8];
    char month[4];
    char day[4];
    unsigned int areaid;
    unsigned int batch;
    unsigned int index;
    unsigned int number;
    unsigned int sn;
    volatile   int status;
    unsigned int datalen1;
    unsigned int datalen2;
    unsigned long frames;
    unsigned long frametime;
    char pos[128];
    unsigned int h;
    unsigned int w;
    unsigned int c;
    unsigned int s;
    float data1[(SHM_VALUE_SIZE-208)/(2*4)];
    float data2[(SHM_VALUE_SIZE-208)/(2*4)];
}IMAGEINFO;*/

typedef struct
{
    char year[8];
    char month[4];
    char day[4];
    unsigned int areaid;
    unsigned int batch;
    unsigned int index;
    unsigned int number;
    unsigned int sn;
    volatile   int status;
    unsigned int datalen1;
    unsigned int datalen2;
    unsigned long frames;
    unsigned long frametime;
    char pos[128];
    unsigned int h;
    unsigned int w;
    unsigned int c;
    unsigned int s;
    unsigned char data1[(SHM_VALUE_SIZE-208)/2];
    unsigned char data2[(SHM_VALUE_SIZE-208)/2];
}IMAGEINFO;

typedef struct {
    int w;
    int h;
    int c;
    float *data;
} image;

void sig_handler(int signo)
{
    if (signo == SIGUSR1 || signo == SIGUSR2)
    {
        sigFlag = 1;
    }
}

void tell_wait()
{
    sigemptyset(&newset);
    sigemptyset(&zeroset);
    sigaddset(&newset, SIGUSR1);
    sigaddset(&newset, SIGUSR2);

    struct sigaction action;
    action.sa_handler = sig_handler;

    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    if (sigaction(SIGUSR1, &action, NULL) < 0)
    {
        printf("sigaction error\n");
        exit(-1);
    }

    if (sigaction(SIGUSR2, &action, NULL) < 0)
    {
        printf("sigaction error\n");
        exit(-1);
    }

    if (sigprocmask(SIG_BLOCK, &newset, NULL) < 0)
    {
        printf("sigprocmask error\n");
        exit(-1);
    }
}

void tell_parent(pid_t pid)
{
    kill(pid, SIGUSR2);
}

void wait_parent()
{
    while(sigFlag == 0)
    {
        sigsuspend(&zeroset);
    }

    sigFlag = 0;

    if(sigprocmask(SIG_BLOCK, &newset, NULL) < 0)
    {
        printf("set sigprocmask error\n");
        exit(-1);
    }
}

void tell_child(pid_t pid)
{
    kill(pid, SIGUSR1);
}

void wait_child()
{
    while(sigFlag == 0)
    {
        sigsuspend(&zeroset);
    }

    sigFlag = 0;

    if(sigprocmask(SIG_BLOCK, &newset, NULL) < 0)
    {
        printf("set sigprocmask error\n");
        exit(-1);
    }
}

void *open_video_stream(const char *f, int c, int w, int h, int fps)
{
    cv::VideoCapture *cap;

    if(f)
        cap = new cv::VideoCapture(f);
    else
        cap = new cv::VideoCapture(c);

    if(!cap->isOpened())
        return 0;

    if(w)
        cap->set(CV_CAP_PROP_FRAME_WIDTH, w);

    if(h)
        cap->set(CV_CAP_PROP_FRAME_HEIGHT, w);

    if(fps)
        cap->set(CV_CAP_PROP_FPS, w);

    return (void *) cap;
}

double what_time_is_it_now()
{
    struct timeval time;

    if (gettimeofday(&time,NULL))
    {
        return 0;
    }

    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}

/*image make_empty_image(int w, int h, int c, IMAGEINFO *p)
{
    image out;
    out.data = p->data1;
    out.h = h;
    out.w = w;
    out.c = c;
    p->h = h;
        p->w = w;
        p->c = c;

    //memcpy(p,&out,3*sizeof(int));

    return out;
}

image make_image(int w, int h, int c, IMAGEINFO *p)
{
    image out = make_empty_image(w,h,c,p);
    //out.data = calloc(h*w*c, sizeof(float));
    return out;
}

image ipl_to_image(IplImage* src,IMAGEINFO *p)
{
    int h = src->height;
    int w = src->width;
    int c = src->nChannels;
    image im = make_image(w, h, c, p);
    unsigned char *data = (unsigned char *)src->imageData;
    int step = src->widthStep;
    int i, j, k;

    for(i = 0; i < h; ++i){
        for(k= 0; k < c; ++k){
            for(j = 0; j < w; ++j){
                im.data[k*w*h + i*w + j] = data[i*step + j*c + k]/255.;
            }
        }
    }
    return im;
}

void rgbgr_image(image im)
{
    int i;
    for(i = 0; i < im.w*im.h; ++i){
        float swap = im.data[i];
        im.data[i] = im.data[i+im.w*im.h*2];
        im.data[i+im.w*im.h*2] = swap;
    }
}

image mat_to_image(cv::Mat m,IMAGEINFO *p)
{
    IplImage ipl = m;
    image im = ipl_to_image(&ipl,p);
    rgbgr_image(im);
    return im;
}

image copy_image(image p)
{
    image copy = p;
    copy.data = (float*)calloc(p.h*p.w*p.c, sizeof(float));
    memcpy(copy.data, p.data, p.h*p.w*p.c*sizeof(float));
    return copy;
}

void constrain_image(image im)
{
    int i;
    for(i = 0; i < im.w*im.h*im.c; ++i){
        if(im.data[i] < 0) im.data[i] = 0;
        if(im.data[i] > 1) im.data[i] = 1;
    }
}

IplImage *image_to_ipl(image im)
{
    int x,y,c;
    IplImage *disp = cvCreateImage(cvSize(im.w,im.h), IPL_DEPTH_8U, im.c);
    int step = disp->widthStep;
    for(y = 0; y < im.h; ++y){
        for(x = 0; x < im.w; ++x){
            for(c= 0; c < im.c; ++c){
                float val = im.data[c*im.h*im.w + y*im.w + x];
                disp->imageData[y*step + x*im.c + c] = (unsigned char)(val*255);
            }
        }
    }
    return disp;
}

void free_image(image m)
{
    if(m.data){
        free(m.data);
    }
}

cv::Mat image_to_mat(image im)
{
    image copy = copy_image(im);
    constrain_image(copy);
    if(im.c == 3) rgbgr_image(copy);

    IplImage *ipl = image_to_ipl(copy);
    cv::Mat m = cv::cvarrToMat(ipl, true);
    cvReleaseImage(&ipl);
    free_image(copy);
    return m;
}
*/
static void *image_show(void *arg)
{
    int count = 0;
    long framenum = 0;

    IPC_shm *ishm = (IPC_shm*)arg;

    while(!demo_done)
    {
        int i = framenum%SHM_QUEUE_NAME;
        framenum++;

        if( i == 0 )
            count++;

        if( count > SHM_NAME_NUM )
            count = 1;

        printf("image_show i=%d,count=%d\n",i,count);

        SHM_STRUCT *p_addr = ishm[i].siGet_Start_ByPos(count-1);
        IMAGEINFO *p_imageinfo = (IMAGEINFO *)p_addr->shm_value;

        while(1)
        {
            if( p_imageinfo->status == 3)
            {
                if(p_imageinfo->datalen2 > 0)
                {
                    cv::Mat m(p_imageinfo->w,p_imageinfo->h,CV_8UC3);
                    m.data = p_imageinfo->data2;
                    m = m.reshape(0, p_imageinfo->h);
                    cv::imshow("detection", m);
                }
                else
                {
                    // The conversion here is very slow
                    /*image im;
                    im.h = p_imageinfo->h;
                    im.w = p_imageinfo->w;
                    im.c = p_imageinfo->c;
                    im.data = p_imageinfo->data1;
                    printf("w=%d,h=%d,c=%d\n",im.w,im.h,im.c);
                    cv::Mat m = image_to_mat(im);
                    cv::imshow("detection", m);*/

                    cv::Mat m(p_imageinfo->w,p_imageinfo->h,CV_8UC3);
                    m.data = p_imageinfo->data1;
                    m = m.reshape(0, p_imageinfo->h);
                    cv::imshow("detection", m);


                    //cv::imshow("detection", *((cv::Mat*)p_imageinfo->data1));
                }

                cv::waitKey(1);

                p_imageinfo->datalen2 = 0;
                p_imageinfo->status = 0;
            }
            else
            {
                //printf("show: status = %d, count = %d.\n",p_imageinfo->status,count);
                sleep(0);
                continue;
            }

            break;
        }
    }

    return NULL;
}


// g++ simplest_rtmp_receive.cpp -o simplest_rtmp_receive IPC_shm.o -std=c++0x -Wall -fpermissive -Wcpp -I /usr/include/jsoncpp `pkg-config --libs --cflags opencv` -lcurl -L /usr/local/lib -ljsoncpp
int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        printf("usage: %s rtmpurl\n",argv[0]);
        return 0;
    }

    printf("SHM_STRUCT size %ld\n",sizeof(SHM_STRUCT));
    printf("IMAGEINFO size %ld\n",sizeof(IMAGEINFO));

    int pid;

    tell_wait();

    pid = fork();
    if(pid > 0)
    {
        key_t key;
        IPC_shm ishm[SHM_QUEUE_NAME];

        printf("create shm start...%ld\n",time(0));
        for(int i = 0; i < SHM_QUEUE_NAME; i++)
        {
            const char* name = shm_name[i];
            key = ftok(name,0);
            printf("name=%s,key=%d\n",name,key);

            ishm[i].siCreateShm(key);	//anonymous
            ishm[i].siAttShm(NULL,0);
            ishm[i].siInitShm();
        }

        printf("create shm ok...%ld\n",time(0));

        tell_child(pid);

        while(1)
        {
            usleep(10);
        }
        }
    else if (pid == 0)
    {
        wait_parent();

        key_t key;
        IPC_shm ishm[SHM_QUEUE_NAME];

        printf("attach shm start...%ld\n",time(0));
        for(int i = 0; i < SHM_QUEUE_NAME; i++)
        {
            const char* name = shm_name[i];
            key = ftok(name,0);

            ishm[i].siOpenShm(key);
            ishm[i].siAttShm(NULL,0);
        }

        printf("attach shm ok...%ld\n",time(0));

        pthread_t etThread;
        pthread_attr_t etPthread_attr;	// The thread attribute
        pthread_attr_init(&etPthread_attr);
        pthread_attr_setdetachstate(&etPthread_attr, PTHREAD_CREATE_DETACHED);
        pthread_create(&etThread,&etPthread_attr,image_show,(void *)&ishm);

        while(1)
        {
            cap = open_video_stream(argv[1], 0, 0, 0, 0);

            if(!cap)
            {
                printf("Couldn't connect to %s.\n",argv[1]);
                usleep(1000);
                continue;
            }

            break;
        }

        float frametime = 0;
        int count = 0;
        long framenum = 0;
        while(!demo_done)
        {
            demo_time = what_time_is_it_now();

            cv::VideoCapture *p = (cv::VideoCapture *)cap;
            cv::Mat m;

            *p >> m;

            if(m.empty())
            {
                //demo_done = 1;
                //break;
                usleep(1000);
                printf("m is empty!\n");
                frame_time = frametime;

                ((cv::VideoCapture *)cap)->release();

                while(1)
                {
                    cap = open_video_stream(argv[1], 0, 0, 0, 0);

                    if(!cap)
                    {
                        printf("Couldn't connect to %s.\n",argv[1]);
                        usleep(1000);
                        continue;
                    }

                    break;
                }

                continue;
            }

            fps = 1./(what_time_is_it_now() - demo_time);
            demo_time = what_time_is_it_now();

            frametime = frame_time + p->get(CV_CAP_PROP_POS_MSEC);

            //printf("fps=%f,frametime=%f\n",fps,frametime);

            int i = framenum%SHM_QUEUE_NAME;
            framenum++;

            if( i == 0 )
                count++;

            if( count > SHM_NAME_NUM )
                count = 1;

            printf("i=%d,count=%d\n",i,count);

            SHM_STRUCT *p_addr = ishm[i].siGet_Start_ByPos(count-1);
            IMAGEINFO *p_imageinfo = (IMAGEINFO *)p_addr->shm_value;

            while(1)
            {
                if( p_imageinfo->status == 0)
                {
                    p_addr->shm_id = count - 1;
                    snprintf(p_addr->shm_name,SHM_NAME_SIZE,"%ld",framenum);


                    strncpy(p_imageinfo->year,"2018",8);
                    strncpy(p_imageinfo->month,"11",4);
                    strncpy(p_imageinfo->day,"11",4);
                    p_imageinfo->areaid = 101;
                    p_imageinfo->batch = 1;
                    p_imageinfo->index = 0;
                    p_imageinfo->number = count - 1;
                    p_imageinfo->sn = i;
                    p_imageinfo->datalen1 = 1;
                    p_imageinfo->datalen2 = 0;
                    p_imageinfo->frames = framenum - 1;
                    p_imageinfo->frametime = frametime;

                    // The conversion here is very slow
                    /*mat_to_image(m,p_imageinfo);*/

                    // no shm
                    //m.copyTo(*((cv::Mat*)p_imageinfo->data1));

                    IplImage ipl = m;
                    p_imageinfo->h = ipl.height;
                    p_imageinfo->w = ipl.width;
                    p_imageinfo->c = ipl.nChannels;
                    p_imageinfo->s = ipl.widthStep;
                    m = m.reshape(0, 1);
                    memcpy(p_imageinfo->data1,m.data,p_imageinfo->h*p_imageinfo->w*p_imageinfo->c);

                    p_imageinfo->status = 1;
                }
                else
                {
                    //printf("status = %d, count = %d.\n",p_imageinfo->status,count);
                    sleep(0);
                    continue;
                }

                break;
            }
        }

        return 0;
    }
}


