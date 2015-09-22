/*
 * skillcache.cpp
 *
 * Copyright (C) 2006 Atomic Blue (info@planeshift.it, http://www.atomicblue.org)
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation (version 2 of the License)
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include <psconfig.h>

#include "util/skillcache.h"
#include "net/message.h"

//-------------------------------------------------------------------
int count = 0;
psSkillCacheItem::psSkillCacheItem(psSkillCacheItem *item)
    : removed(false), modified(true)
{
    skillId = item->skillId;
    nameId = item->nameId;
    removed = item->removed;
    if (!removed)
    {
        rank = item->rank;
        actualStat = item->actualStat;
        knowledge = item->knowledge;
        knowledgeCost = item->knowledgeCost;
        practice = item->practice;
        practiceCost = item->practiceCost;
        category = item->category;
        stat = item->stat;
    }
}

psSkillCacheItem::psSkillCacheItem(unsigned int nameId)
    : removed(false), modified(true)
{
//printf("skillcache 49 skillid %d \n",nameId);
    this->nameId = nameId;
    this->skillId = nameId;
}

psSkillCacheItem::psSkillCacheItem(int skillId,
                                   unsigned int nameId,
                                   long long R,
                                   long long AS,
                                   long long Y,
                                   long long YC,
                                   long long Z,
                                   long long ZC,
                                   unsigned short CAT,
                                   bool stat)
    : removed(false), modified(true)
{
    this->skillId = skillId;
    this->nameId = nameId;
    rank = R;
    actualStat = AS;
    knowledge = Y;
    knowledgeCost = YC;
    practice = Z;
    practiceCost = ZC;
    category = CAT;
    this->stat = stat;
    modified = true;
//printf("skillcache 77 skillid %d ,  Rank R %d \n",nameId,R);


}

psSkillCacheItem::~psSkillCacheItem()
{
}

unsigned short psSkillCacheItem::size() const
{
    if (removed)
    {
        return sizeof(uint8_t);
    }
    else
    {
        return sizeof(uint8_t) + 7*sizeof(uint16_t) + sizeof(bool);
    }
}

void psSkillCacheItem::update(long long R,
                              long long AS,
                              long long Y,
                              long long YC,
                              long long Z,
                              long long ZC)
{
    if (rank != R)
    {
//printf(" 103 cacheitem rank not = R \n");
        rank = R;
        modified = true;
    }

    if (actualStat != AS)
    {
//printf(" 110 cacheitem actual stat not = AS \n");
        actualStat = AS;
        modified = true;
    }

    if (knowledge != Y)
    {
        knowledge = Y;
        modified = true;
    }

    if (knowledgeCost != YC)
    {
        knowledgeCost = YC;
        modified = true;
    }

    if (practice != Z)
    {
        practice = Z;
        modified = true;
    }

    if (practiceCost != ZC)
    {
        practiceCost = ZC;
        modified = true;
    }
}

void psSkillCacheItem::update(psSkillCacheItem *item)
{
    if (rank != item->rank)
    {
        rank = item->rank;
        modified = true;
    }

    if (actualStat != item->actualStat)
    {
        actualStat = item->actualStat;
        modified = true;
    }

    if (knowledge != item->knowledge)
    {
        knowledge = item->knowledge;
        modified = true;
    }

    if (knowledgeCost != item->knowledgeCost)
    {
        knowledgeCost = item->knowledgeCost;
        modified = true;
    }

    if (practice != item->practice)
    {
        practice = item->practice;
        modified = true;
    }

    if (practiceCost != item->practiceCost)
    {
        practiceCost = item->practiceCost;
        modified = true;
    }
}

void psSkillCacheItem::read(MsgEntry *msg)
{
    psSkillCacheItem::Oper op = (psSkillCacheItem::Oper)msg->GetUInt8();
    switch (op)
    {
        case psSkillCacheItem::REMOVE:
            removed = true;
            break;
        case psSkillCacheItem::UPDATE_OR_ADD:
//printf(" skillcache 188 case  read \n");
            removed = false;
            rank = msg->GetUInt64();
            actualStat = msg->GetUInt64();
            knowledge = msg->GetUInt64();
            knowledgeCost = msg->GetUInt64();
            practice = msg->GetUInt64();
            practiceCost = msg->GetUInt64();
            category = msg->GetUInt64();
            stat = msg->GetBool();
//printf(" 202 read skillcache rank %d, AS %d, prac %d, knw %d \n",rank,actualStat,practice,knowledge);
            break;
    }
    modified = true;
}

void psSkillCacheItem::write(MsgEntry *msg)
{
    if (removed)
    {
        msg->Add((uint8_t)psSkillCacheItem::REMOVE);
    }
    else
    {
        msg->Add((uint8_t)psSkillCacheItem::UPDATE_OR_ADD);
//printf(" skillcache 188 case  WRITE \n");
        msg->Add((uint64_t)rank);
        msg->Add((uint64_t)actualStat);
        msg->Add((uint64_t)knowledge);
        msg->Add((uint64_t)knowledgeCost);
        msg->Add((uint64_t)practice);
        msg->Add((uint64_t)practiceCost);
        msg->Add((uint64_t)category);
        msg->Add(stat);
//printf(" 226 write skillcache rank %d, AS %d, prac %d, knw %d \n",rank,actualStat,practice,knowledge);
    }
    modified = false;
}

//-------------------------------------------------------------------

psSkillCache::psSkillCache()
    : modified(false), progressionPoints(0), newList(false)
{
}

psSkillCache::~psSkillCache()
{
    clear();
}

void psSkillCache::clear()
{
    while (!skillCache.IsEmpty())
    {
        delete skillCache.Front();
        skillCache.PopFront();
    }
}

void psSkillCache::apply(psSkillCache *list)
{
    // If this is a completely new list of skills, clear own internal cache
    if (list->isNew())
        clear();

    newList = list->newList;
    progressionPoints = list->progressionPoints;

    // Apply individual cache items in the received list of skills
    psSkillCacheIter p = list->iterBegin();
    while (p.HasNext())
    {

        psSkillCacheItem *second = p.Next();
        psSkillCacheItem *item = getItemBySkillId(second->getSkillId());

        if (second->isRemoved())
        {
            // If the skill was removed, remove from the cache
            if (item)
            {
                deleteItem(item);
                modified = true;
            }
        }
        else
        {
            // Otherwise either update or add to the cache
            if (item)
            {
                item->update(second);
            }
            else
            {
                item = new psSkillCacheItem(second);
                skillCache.PushBack(item);
//printf("skillcache 287 push  \n");
            }
        }
    }

    modified = true;
}

void psSkillCache::addItem(int /*skillId*/, psSkillCacheItem *item)
{
    skillCache.PushBack(item);
//printf("skillcache 298 push \n");
}

psSkillCacheItem *psSkillCache::getItemBySkillId(uint id)
{
    psSkillCacheIter p(skillCache);
    while (p.HasNext())
    {
        psSkillCacheItem *item = p.Next();
        if (item && (uint)item->getSkillId() == id)
            return item;
    }
    return NULL;
}

void psSkillCache::setModified(bool modified)
{
    this->modified = modified;

    psSkillCacheIter p(skillCache);
    while (p.HasNext())
    {
        psSkillCacheItem *item = p.Next();
        if (item)
        {
            item->setModified(modified);
        }
    }
}

bool psSkillCache::isModified()
{
    if (modified)
    {
        return true;
    }
    
    psSkillCacheIter p(skillCache);
    while (!modified && p.HasNext())
    {
        psSkillCacheItem *item = p.Next();
        if (item && item->isModified())
            return true;
    }
    return false;
}

void psSkillCache::setRemoved(bool removed)
{
    this->removed = removed;
}

bool psSkillCache::hasRemoved()
{
    return removed;
}

void psSkillCache::setProgressionPoints(unsigned int points)
{
    if (points != progressionPoints)
    {
        progressionPoints = points;
        modified = true;
    }
}

unsigned short psSkillCache::size() const
{
    unsigned short sz = sizeof(uint64_t) + sizeof(bool) + sizeof(uint64_t);
    psSkillCacheIter p(skillCache);
    while (p.HasNext())
    {
        psSkillCacheItem *item = p.Next();
        if (item && item->isModified())
            sz += item->size() + (7*sizeof(uint64_t));
    }
    return sz;
}

unsigned short psSkillCache::count() const
{
    unsigned short cnt = 0;
    psSkillCacheIter p(skillCache);
    while (p.HasNext())
    {
        psSkillCacheItem *item = p.Next();
        if (item && item->isModified())
            ++cnt;
    }
    return cnt;
}

void psSkillCache::read(MsgEntry *msg)
{
    unsigned short count;

    progressionPoints = msg->GetUInt32();
    newList = msg->GetBool();
    count = msg->GetUInt16();
    for (int i = 0; i < count; i++)
    {
        unsigned int nameId;
        psSkillCacheItem *item;

        nameId = msg->GetUInt32();
        if ((item = getItemBySkillId(nameId)) == NULL)
        {
            item = new psSkillCacheItem(nameId);
            skillCache.PushBack(item);
        }
        item->read(msg);
    }

    modified = true;
}

void psSkillCache::write(MsgEntry *msg)
{
    msg->Add((uint32_t)progressionPoints);
//printf("skillcache 417 add pp \n");
    msg->Add(newList);
    unsigned short cnt = count();
    msg->Add((uint16_t)cnt);

    // Iterate through the list
    psSkillCacheIter p(skillCache);
    psSkillCacheItem *item = NULL;
    if (p.HasNext())
        item = p.Next();
    while (item)
    {
        if (item->isModified())
        {
            // Add modified skills to the message
            msg->Add(item->getNameId());
            item->write(msg);
            if (item->isRemoved())
            {
                // Remove skills that were marked for removal
                skillCache.Delete(p);
                delete item;
            }
        }
        if (p.HasNext())
            item = p.Next();
        else
            item = NULL;
    }

    newList = false;
    modified = false;
}

csString psSkillCache::ToString() const
{
    csString result;
    
    result.AppendFmt("PP: %d New: %s C: %d - ",progressionPoints,newList?"Yes":"No",count());

    psSkillCacheIter p(skillCache);

    while (p.HasNext())
    {
        psSkillCacheItem* item =  p.Next();
        
        if (item->isRemoved())
        {
            result.AppendFmt("NID: %d Removed;", item->getNameId());
        }
        else
        {
            result.AppendFmt("NID: %d Cat: %d R: %lld AS: %lld K: %lld KC: %lld P: %lld PC: %lld;",
                             item->getNameId(), item->getCategory(), item->getRank(),
                             item->getActualStat(), item->getKnowledge(),
                             item->getKnowledgeCost(), item->getPractice(),
                             item->getPracticeCost());

                             
        }

        if (p.HasNext())
        {
            result.Append(" ");
        }
    }
    return result;
}


bool psSkillCache::deleteItem(psSkillCacheItem *item)
{
    psSkillCacheIter p(skillCache);

    while (p.HasNext())
    {
        if (p.Next() == item)
        {
            skillCache.Delete(p);
            delete item;
            removed = true;
            return true;
        }
    }
    return false;
}
