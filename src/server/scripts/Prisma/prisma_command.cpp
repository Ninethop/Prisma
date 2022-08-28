
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
        static ChatCommandTable prismaMoveCommandTable =
        {
            { "info",       HandleMoveInformation,      rbac::RBAC_PERM_COMMAND_GM,     Console::No },
        };

        static ChatCommandTable gmCommandTable =
        {
            { "list",       HandlePrismaList,           rbac::RBAC_PERM_COMMAND_GM,        Console::No },
            { "invoke",     HandlePrismaInvoke,         rbac::RBAC_PERM_COMMAND_GM,        Console::No },
            { "health",     HandlePrismaHealth,         rbac::RBAC_PERM_COMMAND_GM,        Console::No },
            { "move",       prismaMoveCommandTable },
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

    static bool HandlePrismaHealth(ChatHandler* handler, Optional<bool> enableArg)
    {
        Player* player = handler->GetSession()->GetPlayer();

        PrismaDatabasePreparedStatement* stmt = PrismaDatabase.GetPreparedStatement(PRISMA_SEL_PLAYER_PRISMA);
        stmt->setUInt32(0, player->GetGUID());

        PreparedQueryResult result = PrismaDatabase.Query(stmt);

        if (result)
        {
            for (int i = 0; i < 6; ++i)
            {
                int guid = (*result)[i].GetUInt32();
                if (guid <= 0)
                    continue;

                Prisma* _prisma = new Prisma();
                if (!_prisma->InitializePrismaFromGuid(guid))
                    continue;

                _prisma->RestoreStamina();
                _prisma->RestoreMove();
                _prisma->RestoreStatus();
                _prisma->SavePrismaToDB();

                //_prisma->CleanupsBeforeDelete();
                delete _prisma;
            }

            return true;
        }

        return false;
    }

    static bool HandleMoveInformation(ChatHandler* handler, uint32 move_id)
    {
        const PrismaMoveTemplate* _move = sObjectMgr->GetPrismaMoveTemplate(move_id);
        if (!_move)
        {
            std::string data = "Move with id " + std::to_string(move_id) + " doesn't exist.";
            handler->SendSysMessage(std::string_view{ data });
            return true;
        }

        handler->SendSysMessage(std::string("=== " + _move->Name + " (" + std::to_string(_move->Entry) + ") ==="));
        handler->SendSysMessage(std::string("Spell trigger : " + std::to_string(_move->SpellTrigger)));
        handler->SendSysMessage(std::string("Type : " + std::to_string(uint32(_move->Type)) + ", Category : " + std::to_string(uint32(_move->Category))));
        handler->SendSysMessage(std::string("PP : " + std::to_string(_move->PowerPoints) + ", Base Power : " + std::to_string(_move->BasePower)));
        handler->SendSysMessage(std::string("Accuracy : " + std::to_string(_move->Accuracy) + ", CritRate : " + std::to_string(_move->CritRate)));
        handler->SendSysMessage(std::string("Speed priority : " + std::to_string(_move->SpeedPriority) + ", Selection type : " + std::to_string(uint32(_move->SelectionType))));
        handler->SendSysMessage(std::string("NonVolatile flag : " + std::to_string(_move->NonVolatileStatusFlags) + ", Proba : " + std::to_string(_move->ProbabilityNVSF)));
        handler->SendSysMessage(std::string("Volatile flag : " + std::to_string(_move->VolatileStatusFlags) + ", Proba : " + std::to_string(_move->ProbabilityVSF)));
        handler->SendSysMessage(std::string("VolatileCombat flag : " + std::to_string(_move->VolatileCombatStatusFlags) + ", Proba : " + std::to_string(_move->ProbabilityVCSF)));
        return true;
    }
};

void AddSC_prisma_command()
{
    new PrismaCommand();
}
