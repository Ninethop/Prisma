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
