/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "PrismaDatabase.h"
#include "MySQLPreparedStatement.h"

void PrismaDatabaseConnection::DoPrepareStatements()
{
    if (!m_reconnecting)
        m_stmts.resize(MAX_PRISMADATABASE_STATEMENTS);

    PrepareStatement(PRISMA_SEL_PLAYER_PRISMA, "SELECT Prisma_1, Prisma_2, Prisma_3, Prisma_4, Prisma_5, Prisma_6 FROM player_prisma WHERE Guid = ?", CONNECTION_SYNCH);
    PrepareStatement(PRISMA_SEL_PRISMA_ID_FROM_GUID, "SELECT ID FROM prisma WHERE Guid = ?", CONNECTION_SYNCH);
    PrepareStatement(PRISMA_DEL_PRISMA, "DELETE FROM prisma WHERE Guid = ?", CONNECTION_ASYNC);
    PrepareStatement(PRISMA_INS_PRISMA, "INSERT INTO prisma (Guid, ID, Gender, Nature, Level, Experience, Item, IV_Stamina, IV_Attack, IV_Defense, IV_SpecialAttack, IV_SpecialDefense, IV_Speed, EV_Stamina, EV_Attack, EV_Defense, EV_SpecialAttack, EV_SpecialDefense, EV_Speed, Move_0, PP_Move_0, Move_1, PP_Move_1, Move_2, PP_Move_2, Move_3, PP_Move_3) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(PRISMA_SEL_PRISMA_FROM_GUID, "SELECT Guid, ID, Gender, Nature, Level, Experience, Item, IV_Stamina, IV_Attack, IV_Defense, IV_SpecialAttack, IV_SpecialDefense, IV_Speed, EV_Stamina, EV_Attack, EV_Defense, EV_SpecialAttack, EV_SpecialDefense, EV_Speed, Move_0, PP_Move_0, Move_1, PP_Move_1, Move_2, PP_Move_2, Move_3, PP_Move_3 FROM prisma WHERE Guid = ?", CONNECTION_SYNCH);
}

PrismaDatabaseConnection::PrismaDatabaseConnection(MySQLConnectionInfo& connInfo) : MySQLConnection(connInfo)
{
}

PrismaDatabaseConnection::PrismaDatabaseConnection(ProducerConsumerQueue<SQLOperation*>* q, MySQLConnectionInfo& connInfo) : MySQLConnection(q, connInfo)
{
}

PrismaDatabaseConnection::~PrismaDatabaseConnection()
{
}
