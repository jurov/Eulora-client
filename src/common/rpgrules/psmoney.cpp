
/*
 * psmoney.cpp by Anders Reggestad <andersr@pvv.org>
 *
 * Copyright (C) 2001 Atomic Blue (info@planeshift.it, http://www.atomicblue.org) 
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
 *
 */

#include <cssysdef.h>
#include <csgeom/math.h>
#include <psconfig.h>

#include "psmoney.h"
#include "util/log.h"


psMoney::psMoney()
    :bitcents(0), denarius(0), argents(0), coppers(0)
{
}

psMoney::psMoney(int coppers)
    :bitcents(0), denarius(0), argents(0), coppers(coppers)
{
}

psMoney::psMoney(int bitcents, int denarius, int argents, int coppers)
    :bitcents(bitcents), denarius(denarius), argents(argents), coppers(coppers)
{
}

psMoney::psMoney(const char * moneyString)
    :bitcents(0), denarius(0), argents(0), coppers(0)
{
    Set(moneyString);
}

void psMoney::Set(const char * moneyString)
{
    // This constructor gets called from a message cracker, we cannot presume moneyString will be valid
    if (moneyString==NULL)
        return;

    if (sscanf(moneyString,"%d,%d,%d,%d",&bitcents,&denarius,&argents,&coppers) != 4)
    {
        bitcents = denarius = argents = 0;
        if (sscanf(moneyString,"%d",&coppers) != 1)
        {
            coppers = 0;
        }
    }
}

void psMoney::Set( int type, int value )
{
    switch( type )
    {
        case MONEY_COPPERS:   SetCoppers( value ); break;
        case MONEY_ARGENTS:   SetArgents( value ); break;
        case MONEY_DENARIUS:   SetDenarius( value ); break;
        case MONEY_BITCENTS: SetBITCents( value ); break;
    }
}

void psMoney::Set(int bitcents, int denarius, int argents, int coppers)
{
    this->bitcents  =  bitcents;
    this->denarius    =  denarius;
    this->argents    =  argents;
    this->coppers    =  coppers;
}

int psMoney::GetTotal() const
{
    int64 total = (int64)bitcents*BITCENTS_VALUE_COPPERS + (int64)denarius*DENARIUS_VALUE_COPPERS
        + (int64)argents*ARGENTS_VALUE_COPPERS + coppers;
    if(total > INT_MAX)
        total = INT_MAX;
    return (int)total;
}

csString psMoney::ToString() const
{ 
    return csString().Format("%d,%d,%d,%d",bitcents,denarius,argents,coppers);
}

csString psMoney::ToUserString() const
{
    if(!coppers && !argents && !denarius && !bitcents)
        return csString("0 Coppers");

    csString strs[4];
    int found = 0;

    if(bitcents)
    {
      strs[0] = bitcents;
      strs[0].Append(" BITCents");
      found = 1;
    }
    if(denarius)
    {
      strs[found] = denarius;
      strs[found].Append(" Denarius");
      found++;
    }
    if(argents)
    {
      strs[found] = argents;
      strs[found].Append(" Argents");
      found++;
    }
    if(coppers)
    {
      strs[found] = coppers;
      strs[found].Append(" Coppers");
      found++;
    }

    csString temp((size_t)found * 11);
    temp = strs[0];
    if(found > 1)                 // Only add "and" if there's at least two entries
    {
        const int fMinus1 = found - 1;
        int i = 1;
        for(; i < fMinus1; i++)   // Loop either one or two times to add in commas
        {
            temp.Append(", ");
            temp.Append(strs[i]);
        }
        temp.Append(" and ");
        temp.Append(strs[i]);
    }
    return temp;
}

void psMoney::Adjust( int type, int value )
{
    switch( type )
    {
        case MONEY_COPPERS:   AdjustCoppers(  value ); break;
        case MONEY_ARGENTS:   AdjustArgents(  value ); break;
        case MONEY_DENARIUS:   AdjustDenarius(  value ); break;
        case MONEY_BITCENTS: AdjustBITCents(value ); break;
    }
}


void psMoney::AdjustBITCents( int c)
{ 
	bitcents+= c; 
	if ( bitcents < 0 )
		bitcents = 0;
}

void psMoney::AdjustDenarius( int c )
{ 
	denarius+= c; 
	if ( denarius < 0 )
		denarius = 0;
}
void psMoney::AdjustArgents( int c )
{ 
    argents+= c; 
    if ( argents < 0 )
        argents = 0;
}


void psMoney::AdjustCoppers( int c )
{ 
    coppers+= c; 
    if ( coppers < 0 )
        coppers = 0;
}


bool psMoney::EnsureCoppers(int minValue)
{
	int total = GetTotal();

	if (total < minValue)
		return false;

	// Reserve how many trias we need
	total -= minValue;

	// Now normalize the remainder
	Set(0,0,0,total);
	*this = Normalized();

	// Now add back in the trias we need
	coppers += minValue;
	return true;
}

bool psMoney::EnsureArgents(int minValue)
{
	int total = GetTotal();

	if (total < minValue * ARGENTS_VALUE_COPPERS)
		return false;

	// Reserve how many hexas we need
	total -= minValue * ARGENTS_VALUE_COPPERS;

	// Now normalize the remainder
	Set(0,0,0,total);
	*this = Normalized();

	// Now add back in the hexas we need
	argents += minValue;
	return true;
}

bool psMoney::EnsureDenarius(int minValue)
{
	int total = GetTotal();

	if (total < minValue * DENARIUS_VALUE_COPPERS)
		return false;

	// Reserve how many octas we need
	total -= minValue * DENARIUS_VALUE_COPPERS;

	// Now normalize the remainder
	Set(0,0,0,total);
	*this = Normalized();

	// Now add back in the octas we need
	denarius += minValue;

	return true;
}

bool psMoney::EnsureBITCents(int minValue)
{
	int total = GetTotal();

	if (total < minValue * BITCENTS_VALUE_COPPERS)
		return false;

	// Reserve how many circles we need
	total -= minValue * BITCENTS_VALUE_COPPERS;

	// Now normalize the remainder
	Set(0,0,0,total);
	*this = Normalized();

	// Now add back in the hexas we need
	bitcents += minValue;
	return true;
}

int psMoney::Get( int type ) const
{
    switch( type )
    {
        case MONEY_COPPERS:   return coppers;
        case MONEY_ARGENTS:   return argents;
        case MONEY_DENARIUS:   return denarius;
        case MONEY_BITCENTS: return bitcents;
    }
    return 0;
}

psMoney psMoney::Normalized() const
{
    int tot = GetTotal(); 
    int c = tot/BITCENTS_VALUE_COPPERS;
    tot = tot%BITCENTS_VALUE_COPPERS;
    int o = tot/DENARIUS_VALUE_COPPERS;
    tot = tot%DENARIUS_VALUE_COPPERS;
    int h = tot/ARGENTS_VALUE_COPPERS;
    tot = tot%ARGENTS_VALUE_COPPERS;
    int t = tot;
    return psMoney(c,o,h,t);
}


bool psMoney::operator > (const psMoney& other) const
{
    return GetTotal() > other.GetTotal();
}

psMoney psMoney::operator +=(const psMoney& other)
{
    bitcents += other.bitcents;
    denarius += other.denarius;
    argents += other.argents;
    coppers += other.coppers;
    return psMoney(bitcents, denarius, argents, coppers);
}

psMoney psMoney::operator - (void) const
{
    return psMoney(-bitcents, -denarius, -argents, -coppers);
}

psMoney psMoney::operator - (const psMoney& other) const
{
    psMoney left,taken;
    left = *this;

    int type = MONEY_COPPERS;
    while(taken.GetTotal() < other.GetTotal() && type <= MONEY_BITCENTS)
    {
      const int amount = csMin( left.Get(type), other.GetTotal() - taken.GetTotal() );
      taken.Adjust(type, amount);
      left.Adjust(type, -amount);
      type++;
    }
    
    // Now we might have taken a little to much so calculate and give
    // change.
    psMoney change(0,0,0,taken.GetTotal()-other.GetTotal());

    psMoney result = left+change.Normalized();

    if (result.GetTotal() != (GetTotal()-other.GetTotal()))
    {
        Error5("Something is very wrong %d - %d = %d but calculated to be %d",
               GetTotal(),other.GetTotal(),(GetTotal()-other.GetTotal()),result.GetTotal());
        // If the calculation go wrong make sure we don't return anything wrong from this function.
        result = psMoney(0,0,0,GetTotal()-other.GetTotal()).Normalized();
    }

    //    csString str;
    //    str.AppendFmt("Money[%s](%d) - Money[%s](%d) = Money[%s](%d)",
    //                ToString().GetDataSafe(),GetTotal(),other.ToString().GetDataSafe(),other.GetTotal(),
    //                result.ToString().GetDataSafe(),result.GetTotal());
    //  Debug1(LOG_EXCHANGES, str.GetDataSafe());

    return result;
}

psMoney psMoney::operator + (const psMoney& other) const
{
    return psMoney(bitcents+other.bitcents,
                   denarius+other.denarius,
                   argents+other.argents,
                   coppers+other.coppers);
}

psMoney psMoney::operator * (const int mult) const
{
    return psMoney(bitcents*mult,denarius*mult,argents*mult,coppers*mult);
}

