
#include "prisma.h"
#include "PrismaData.h"
#include "ScriptMgr.h"
#include "ObjectMgr.h"
#include "Log.h"
#include "Chat.h"
#include "Player.h"
#include "WorldSession.h"
#include "DatabaseEnv.h"

using namespace Trinity::ChatCommands;

class PrismaCommand : public CommandScript
{
public:
    PrismaCommand() : CommandScript("PrismaCommand") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable gmCommandTable =
        {
            { "list",       HandlePrismaList,           rbac::RBAC_PERM_COMMAND_GM,        Console::No },
            { "invoke",     HandlePrismaInvoke,         rbac::RBAC_PERM_COMMAND_GM,        Console::No },
        };
        static ChatCommandTable commandTable =
        {
            { "prisma", gmCommandTable },
        };
        return commandTable;
    }

    static bool HandlePrismaList(ChatHandler* handler, Optional<bool> enableArg)
    {
        Player* target = handler->getSelectedPlayer();
        if (!target)
            target = handler->GetSession()->GetPlayer();

        int guid = target->GetGUID();

        PrismaDatabasePreparedStatement* stmt = PrismaDatabase.GetPreparedStatement(PRISMA_SEL_PLAYER_PRISMA);
        stmt->setUInt32(0, guid);

        PreparedQueryResult result = PrismaDatabase.Query(stmt);

        int prisma[6];
        if (result)
        {
            int count = 0;
            for (int i = 0; i < 6; i++)
            {
                prisma[i] = (*result)[i].GetUInt32();

                if (prisma[i] > 0)
                {
                    count++;
                    PrismaTemplate const* _prisma = sObjectMgr->GetPrismaTemplate(prisma[i]);   
                    TC_LOG_INFO("prisma", "You have the prisma_%u, with GUID %u, is name is : %s.", count, prisma[i], _prisma->Name);                   
                }
            }

            if (count > 0)
                TC_LOG_INFO("prisma", "You have %u prisma", count);

            return true;
        }

        return false;
    }

    /* this command invoke the first Prisma in the player team */
    static bool HandlePrismaInvoke(ChatHandler* handler, Optional<bool> enableArg)
    {
        Player* player = handler->GetSession()->GetPlayer();
        Map* map = player->GetMap();

        PrismaDatabasePreparedStatement* stmt = PrismaDatabase.GetPreparedStatement(PRISMA_SEL_PLAYER_PRISMA);
        stmt->setUInt32(0, player->GetGUID());

        PreparedQueryResult result = PrismaDatabase.Query(stmt);

        if (result)
        {
            int prisma_guid = (*result)[0].GetUInt32();
            if (prisma_guid < 0)
                return false;

            PrismaDatabasePreparedStatement* stmt_id = PrismaDatabase.GetPreparedStatement(PRISMA_SEL_PRISMA_ID_FROM_GUID);
            stmt_id->setUInt32(0, prisma_guid);
            PreparedQueryResult result_id = PrismaDatabase.Query(stmt_id);

            if (result_id)
            {
                int prisma_id = (*result_id)[0].GetUInt32();
                if (prisma_id < 0)
                    return false;

                //PrismaTemplate const* prisma = sObjectMgr->GetPrismaTemplate(prisma_guid);
                Creature* creature = new Creature();

                if (!creature->Create(map->GenerateLowGuid<HighGuid::Unit>(), map, player->GetPhaseMaskForSpawn(), PRISMA_TEMPLATE_RESERVED_MIN + prisma_id, *player))
                {
                    delete creature;
                    return false;
                }

                creature->SaveToDB(map->GetId(), (1 << map->GetSpawnMode()), player->GetPhaseMaskForSpawn());
                ObjectGuid::LowType db_guid = creature->GetSpawnId();
                creature->CleanupsBeforeDelete();
                delete creature;

                creature = new Creature();

                if (!creature->LoadFromDB(db_guid, map, true, true))
                {
                    delete creature;
                    return false;
                }

                sObjectMgr->AddCreatureToGrid(db_guid, sObjectMgr->GetCreatureData(db_guid));
                return true;
            }
        }

        return false;
    }
};

void AddSC_prisma_command()
{
    new PrismaCommand();
}
