#pragma once

#include "MySQLConnection.h"

enum PrismaDatabaseStatements : uint32
{
    /*  Naming standard for defines:
        {DB}_{SEL/INS/UPD/DEL/REP}_{Summary of data changed}
        When updating more than one field, consider looking at the calling function
        name for a suiting suffix.
    */

    PRISMA_SEL_PLAYER_PRISMA,
    PRISMA_SEL_PRISMA_ID_FROM_GUID,
    PRISMA_DEL_PRISMA,
    PRISMA_INS_PRISMA,
    PRISMA_SEL_PRISMA_FROM_GUID,

    MAX_PRISMADATABASE_STATEMENTS
};

class TC_DATABASE_API PrismaDatabaseConnection : public MySQLConnection
{
public:
    typedef PrismaDatabaseStatements Statements;

    //- Constructors for sync and async connections
    PrismaDatabaseConnection(MySQLConnectionInfo& connInfo);
    PrismaDatabaseConnection(ProducerConsumerQueue<SQLOperation*>* q, MySQLConnectionInfo& connInfo);
    ~PrismaDatabaseConnection();

    //- Loads database type specific prepared statements
    void DoPrepareStatements() override;
};
