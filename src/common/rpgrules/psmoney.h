/*
 * psmoney.h by Anders Reggestad <andersr@pvv.org>
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
#ifndef PS_MONEY
#define PS_MONEY

//CS includes
#include <csutil/csstring.h>

// Value of coins in other coins
#define BITCENTS_VALUE_COPPERS 1000000
#define DENARIUS_VALUE_COPPERS  10000
#define ARGENTS_VALUE_COPPERS   100

#define BITCENTS_VALUE_ARGENTS  (BITCENTS_VALUE_COPPERS / ARGENTS_VALUE_COPPERS)
#define BITCENTS_VALUE_DENARIUS   (BITCENTS_VALUE_COPPERS / DENARIUS_VALUE_COPPERS)
#define DENARIUS_VALUE_ARGENTS     (DENARIUS_VALUE_COPPERS / ARGENTS_VALUE_COPPERS)

typedef enum
{
    MONEY_COPPERS,
    MONEY_ARGENTS,
    MONEY_DENARIUS,
    MONEY_BITCENTS
} Money_Slots;


class psMoney 
{
public:
    psMoney();
    psMoney(int coppers);
    psMoney(int bitcents, int denarius, int argents, int coppers);

    /** Construct a psMoney based on a string 
     *  Format: "C,O,H,T"
     */
    psMoney(const char * moneyString);
    
   
    void Set(const char * moneyString);
    void Set(int type, int value);
    void Set(int bitcents, int denarius, int argents, int coppers);

    /** Set the number of circles
     */
    void SetBITCents(int c) { bitcents = c; }
    void AdjustBITCents( int c );
    
    /** Get the number of circles
     * @return circles
     */
    int GetBITCents() const { return bitcents; }

    /** Set the number of octas
     */
    void SetDenarius(int o) { denarius = o; }
    void AdjustDenarius(int o);
    /** Get the number of octas
     * @return octas
     */
    int GetDenarius() const { return denarius; }

    /** Set the number of hexas
     */
    void SetArgents(int h) { argents = h; }
    void AdjustArgents( int h );
    /** Get the number of hexas
     * @return hexas
     */
    int GetArgents() const { return argents; }

    /** Set the number of trias
     */
    void SetCoppers(int t) { coppers = t; }   
    void AdjustCoppers( int t );

    /** Get the number of trias
     * @return trias
     */
    int GetCoppers() const { return coppers; }   
   
    /** Get the total in trias
     * @return Trias
     */
    int GetTotal() const;

    /** Convert psMoney to a string
     * @return "C,O,H,T"
     */
    csString ToString() const;

    /** Convert psMoney to user-friendly string
     * @return "12 Circles, 3 Hexas and 14 Trias"
     */
    csString ToUserString() const;

    /** Normalize to have the total match the highest possible number of high value coins. E.g. 11 trias would normalized be 1 Hexa and 1 Tria.
     * @return The normalized money without modifying the original object.
     */
    psMoney Normalized() const;

    bool operator > (const psMoney& other) const;
    psMoney operator - (const psMoney& other) const;
    psMoney operator - (void) const;
    psMoney operator + (const psMoney& other) const;
    psMoney operator +=(const psMoney& other);
    psMoney operator * (const int mult) const;
    
    void Adjust( int type, int value );
    int Get( int type ) const;

	bool EnsureCoppers(int minValue);
	bool EnsureArgents(int minValue);
	bool EnsureDenarius(int minValue);
	bool EnsureBITCents(int minValue);

protected:
    int bitcents;
    int denarius;
    int argents;
    int coppers;
};


#endif
