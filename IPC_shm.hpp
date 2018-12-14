#ifndef __IPC_SHM_HPP__
#define __IPC_SHM_HPP__

#include <sys/shm.h>
#include <string.h>
#include <iostream>
#include <cstdio>
#include <errno.h>
#include <sys/stat.h>

#define SHM_NAME_NUM 	25
#define SHM_NAME_SIZE 	32
#define SHM_VALUE_SIZE 	1024*1024*30

typedef struct
{
    int shm_id;
    char shm_name[SHM_NAME_SIZE];
    char shm_value[SHM_VALUE_SIZE];
}SHM_STRUCT;

class IPC_shm
{
private:

    key_t skKey;					// 共享内存的标识符
    int siSize;					// 共享内存的大小
    int siId;						// 共享内存的标识ID
    SHM_STRUCT *spAddrStart;		// 连接好的地址

    std::size_t suiState;
    enum seShmState
    {
        err_creat_shm = 1,
        err_open_shm = 2,
        err_att_shm = 4,
        err_ctl_shm = 8,
        err_det_shm = 16
    };

    int siCount;

public:

    IPC_shm();
    ~IPC_shm();

    void svSetState(seShmState state) { suiState |= state; }
    void svResetState() { suiState = 0; }
    bool sbGood() const;

    int siCreateShm(key_t shmkey=IPC_PRIVATE, int shmsize=SHM_NAME_NUM*sizeof(SHM_STRUCT),int shmflag=IPC_CREAT|S_IRUSR|S_IWUSR);
    int siOpenShm(key_t shmkey=IPC_PRIVATE, int shmsize=SHM_NAME_NUM*sizeof(SHM_STRUCT),int shmflag=S_IRUSR|S_IWUSR);
    void *siAttShm(const void *shmaddr=NULL,int shmflag=SHM_RDONLY);
    int siGetShmStat(struct shmid_ds *buf);
    int siSetShmStat(struct shmid_ds *buf);
    int siRmShm();
    int siLockShm();
    int siUnLockShm();
    int siDteShm();
    void siInitShm();

    SHM_STRUCT * siGet_Start_ByPos(int);


    int siGet_Value_ById(int, SHM_STRUCT **);
    int siGet_Value_ByName(const char *, SHM_STRUCT **);
    int siAdd_Value(const SHM_STRUCT *);
    int siDel_Value_ByName(const char *);
    int siDel_Value_ById(int);
    int siUpd_Value_ByName(const char *, const char *);
    int siUpd_Value_ById(int , const char *);
    char *scGet_Value_ByName(const char *, char *);
    char *scGet_Value_ById(int , char *);
    int siCheck_Repeat_ByName(const char *);
    int siCheck_Repeat_ById(int);

    int siPrintValue();
};


#endif

