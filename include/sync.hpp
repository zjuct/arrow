
enum SyncType
{
    Sync_Player,
    Sync_Candy,
    Sync_Arrow,
    Sync_Num,
};

class SyncPackage
{
    SyncType type;
    int size;
    char *data;

    virtual ~SyncPackage()
    {
        if (data)
            delete[] data;
    }
};