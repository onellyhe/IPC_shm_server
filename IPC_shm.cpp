#ifndef __IPC_SHM_HPP__
#include "IPC_shm.hpp"

// g++ -c IPC_shm.cpp
IPC_shm::IPC_shm()
{
    skKey = IPC_PRIVATE;
    siSize = SHM_NAME_NUM*sizeof(SHM_STRUCT);
    siId = -1;
    spAddrStart = NULL;

    suiState = 0;
    siCount = 0;
}

IPC_shm::~IPC_shm()
{
    siDteShm();
}

int IPC_shm::siCreateShm(key_t shmkey,int shmsize,int shmflag)
{
    if((siId = shmget(shmkey,shmsize,shmflag)) == -1 )
    {
        svSetState(err_creat_shm);
            fprintf(stderr, "Create Share Memory Error:%s\n\a", strerror(errno));
            return -1;
        }

    return 0;
}

int IPC_shm::siOpenShm(key_t shmkey,int shmsize,int shmflag)
{
    if((siId = shmget(shmkey,shmsize,shmflag)) == -1 )
    {
        svSetState(err_open_shm);
            fprintf(stderr, "Open Share Memory Error:%s\n\a", strerror(errno));
            return -1;
        }

    return 0;
}

void *IPC_shm::siAttShm(const void *shmaddr,int shmflag)
{
    spAddrStart = (SHM_STRUCT *)shmat(siId, shmaddr, shmflag);
    if(spAddrStart == (void*)-1)
    {
        svSetState(err_att_shm);
        fprintf(stderr, "Attach Share Memory Error:%s\n\a", strerror(errno));
        return NULL;
    }

    return spAddrStart;
}

int IPC_shm::siGetShmStat(struct shmid_ds *buf)
{
    if(buf == NULL)
        return -1;

    if(shmctl(siId,IPC_STAT,buf) == -1 )
    {
        svSetState(err_ctl_shm);
            fprintf(stderr, "Get Share Memory Stat Error:%s\n\a", strerror(errno));
            return -1;
        }

    return 0;
}

int IPC_shm::siSetShmStat(struct shmid_ds *buf)
{
    if(buf == NULL)
        return -1;

    if(shmctl(siId,IPC_SET,buf) == -1 )
    {
        svSetState(err_ctl_shm);
            fprintf(stderr, "Set Share Memory Stat Error:%s\n\a", strerror(errno));
            return -1;
        }

    return 0;
}

int IPC_shm::siRmShm()
{
    if(shmctl(siId,IPC_RMID,NULL) == -1 )
    {
        svSetState(err_ctl_shm);
            fprintf(stderr, "Rm Share Memory Error:%s\n\a", strerror(errno));
            return -1;
        }

    return 0;
}

int IPC_shm::siLockShm()
{
    if(shmctl(siId,SHM_LOCK,NULL) == -1 )
    {
        svSetState(err_ctl_shm);
            fprintf(stderr, "Lock Share Memory Error:%s\n\a", strerror(errno));
            return -1;
        }

    return 0;
}

int IPC_shm::siUnLockShm()
{
    if(shmctl(siId,SHM_UNLOCK,NULL) == -1 )
    {
        svSetState(err_ctl_shm);
            fprintf(stderr, "UnLock Share Memory Error:%s\n\a", strerror(errno));
            return -1;
        }

    return 0;
}

int IPC_shm::siDteShm()
{
    if(spAddrStart == NULL)
        return -1;

    if(shmdt(spAddrStart) == -1 )
    {
        svSetState(err_det_shm);
            fprintf(stderr, "UnLock Share Memory Error:%s\n\a", strerror(errno));
            return -1;
        }

    return 0;
}

bool IPC_shm::sbGood() const
{
    if ( (suiState & (err_creat_shm | err_open_shm
        | err_att_shm | err_ctl_shm | err_det_shm)))
    {
        return false;
    }
    else
    {
        return true;
    }
}

int IPC_shm::siGet_Value_ById(int id, SHM_STRUCT ** value)
{
    int pos;

    if (spAddrStart == NULL)
    {
        return -1;
    }

    for(pos=0; pos<SHM_NAME_NUM; pos++)
    {
        if (spAddrStart[pos].shm_id==id)
        {
            *value = &(spAddrStart[pos]);
            return 0;
        }
    }

    return -2;
}

int IPC_shm::siGet_Value_ByName(const char *name, SHM_STRUCT **value)
{
    int pos;

    if (name == NULL || spAddrStart==NULL)
    {
        return -1;
    }

    for(pos=0; pos<SHM_NAME_NUM; pos++)
    {
        if (strncmp(spAddrStart[pos].shm_name, name,SHM_NAME_SIZE)==0)
        {
            *value = &(spAddrStart[pos]);
            return 0;
        }
    }

    return -2;
}

int IPC_shm::siAdd_Value(const SHM_STRUCT *value)
{
    //int pos;
    int ret;
    SHM_STRUCT *pvalue;

    if(value == NULL)
        return -1;

    if (siCheck_Repeat_ById(value->shm_id) < 0
        || siCheck_Repeat_ByName(value->shm_name) < 0)
        return -2;

    ret = siGet_Value_ById(0, &pvalue);
    if (ret<0) return ret;

    memcpy(pvalue, value, sizeof(SHM_STRUCT) );

    siCount ++;

    return 0;
}

int IPC_shm::siDel_Value_ByName(const char *name)
{
    //int pos;
    int ret;
    SHM_STRUCT *pvalue;

    if(name == NULL)
        return -1;

    ret = siGet_Value_ByName(name, &pvalue);
    if (ret<0) return ret;

    memset(pvalue, 0, sizeof(SHM_STRUCT));

    siCount--;

    return 0;
}

int IPC_shm::siDel_Value_ById(int id)
{
    //int pos;
    int ret;
    SHM_STRUCT *pvalue;

    ret = siGet_Value_ById(id, &pvalue);
    if (ret<0) return ret;

    memset(pvalue, 0, sizeof(SHM_STRUCT));

    siCount--;

    return 0;
}

int IPC_shm::siUpd_Value_ByName(const char *name, const char *value)
{
    //int pos;
    int ret;
    SHM_STRUCT *pvalue;

    if(name == NULL || value==NULL)
        return -1;

    ret = siGet_Value_ByName(name, &pvalue);
    if (ret<0) return ret;

    strcpy(pvalue->shm_value, value);

    return 0;
}

int IPC_shm::siUpd_Value_ById(int id, const char *value)
{
    //int pos;
    int ret;
    SHM_STRUCT *pvalue;

    if(value==NULL)
        return -1;

    ret = siGet_Value_ById(id, &pvalue);
    if (ret<0) return ret;

    strcpy(pvalue->shm_value, value);

    return 0;
}

char *IPC_shm::scGet_Value_ByName(const char *name, char *value)
{
    //int pos;
    int ret;
    SHM_STRUCT *pvalue;
    char *p = value;

    if(name == NULL || value == NULL)
        return NULL;

    ret = siGet_Value_ByName(name, &pvalue);
    if (ret<0) return NULL;

    strcpy(value, pvalue->shm_value);

    return p;
}

char *IPC_shm::scGet_Value_ById(int id, char *value)
{
    //int pos;
    int ret;
    SHM_STRUCT *pvalue;
    char *p = value;

    if(value == NULL)
        return NULL;

    ret = siGet_Value_ById(id, &pvalue);
    if (ret<0) return NULL;

    strcpy(value, pvalue->shm_value);

    return p;
}

int IPC_shm::siCheck_Repeat_ByName(const char *name)
{
    int pos;

    if (spAddrStart == NULL)
    {
        return -1;
    }

    for(pos=0; pos<SHM_NAME_NUM; pos++)
    {
        if (strncmp(spAddrStart[pos].shm_name, name,SHM_NAME_SIZE)==0)
        {
            return -2;
        }
    }

    return 0;
}

int IPC_shm::siCheck_Repeat_ById(int id)
{
    int pos;

    if (spAddrStart==NULL)
    {
        return -1;
    }

    for(pos=0; pos<SHM_NAME_NUM; pos++)
    {
        if(spAddrStart[pos].shm_id == id)
        {
            return -2;
        }
    }

    return 0;
}

int IPC_shm::siPrintValue()
{
    int pos;

    if (spAddrStart == NULL)
    {
        return -1;
    }

    for(pos=0; pos<SHM_NAME_NUM; pos++)
    {
        if(spAddrStart[pos].shm_id != 0)
        {
            std::cout<<"ID = "<<spAddrStart[pos].shm_id
                <<" NAME = "<<spAddrStart[pos].shm_name
                <<" VALUE = "<<spAddrStart[pos].shm_value<<std::endl;
        }
    }

    return 0;
}

SHM_STRUCT * IPC_shm::siGet_Start_ByPos(int pos)
{
    if(pos > SHM_NAME_NUM - 1)
        return NULL;

    return spAddrStart + pos;
}

void IPC_shm::siInitShm()
{
    memset(spAddrStart,0,siSize);
}

#endif

