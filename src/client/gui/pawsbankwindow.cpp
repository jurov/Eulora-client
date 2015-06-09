/*
* pawsbankwindow.cpp - Author: Mike Gist
*
* Copyright (C) 2007 Atomic Blue (info@planeshift.it, http://www.atomicblue.org)
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
#include "globals.h"

#include <csutil/csstring.h>

#include "net/messages.h"
#include "net/clientmsghandler.h"
#include "pawsbankwindow.h"

/* Widget ID's */
#define MONEYBUTTON   1000
#define ADMINBUTTON   1002
#define WITHDRAW      1101
#define DEPOSIT       1102
#define EXCHANGE      1103
#define CIRCLESBUTTON 1104
#define OCTASBUTTON   1105
#define HEXASBUTTON   1106
#define TRIASBUTTON   1107

pawsBankWindow::pawsBankWindow()
{
}

pawsBankWindow::~pawsBankWindow()
{
    psengine->GetMsgHandler()->Unsubscribe(this, MSGTYPE_BANKING);
}

bool pawsBankWindow::PostSetup()
{
    psengine->GetMsgHandler()->Subscribe(this, MSGTYPE_BANKING);

    Money = (pawsButton*)(FindWidget("MoneyButton"));
    if(!Money)
        return false;
    Admin = (pawsButton*)(FindWidget("AdminButton"));
    if(!Admin)
        return false;

    moneyWindow = FindWidget("MoneyWindow");
    if(!moneyWindow)
        return false;
    adminWindow = FindWidget("AdminWindow");
    if(!adminWindow)
        return false;

    bitcentsCanWithdraw = dynamic_cast <pawsTextBox*> (FindWidget("BITCentsCanWithdraw"));
    if(!bitcentsCanWithdraw)
        return false;
    denariusCanWithdraw = dynamic_cast <pawsTextBox*> (FindWidget("DenariusCanWithdraw"));
    if(!denariusCanWithdraw)
        return false;
    argentsCanWithdraw = dynamic_cast <pawsTextBox*> (FindWidget("ArgentsCanWithdraw"));
    if(!argentsCanWithdraw)
        return false;
    coppersCanWithdraw = dynamic_cast <pawsTextBox*> (FindWidget("CoppersCanWithdraw"));
    if(!coppersCanWithdraw)
        return false;
    bitcentsCanDeposit = dynamic_cast <pawsTextBox*> (FindWidget("BITCentsCanDeposit"));
    if(!bitcentsCanDeposit)
        return false;
    denariusCanDeposit = dynamic_cast <pawsTextBox*> (FindWidget("DenariusCanDeposit"));
    if(!denariusCanDeposit)
        return false;
    argentsCanDeposit = dynamic_cast <pawsTextBox*> (FindWidget("ArgentsCanDeposit"));
    if(!argentsCanDeposit)
        return false;
    coppersCanDeposit = dynamic_cast <pawsTextBox*> (FindWidget("CoppersCanDeposit"));
    if(!coppersCanDeposit)
        return false;
    bitcentsToWithdraw = dynamic_cast <pawsEditTextBox*> (FindWidget("BITCentsToWithdraw"));
    if(!bitcentsToWithdraw)
        return false;
    bitcentsToWithdraw->SetText("0");
    denariusToWithdraw = dynamic_cast <pawsEditTextBox*> (FindWidget("DenariusToWithdraw"));
    if(!denariusToWithdraw)
        return false;
    denariusToWithdraw->SetText("0");
    argentsToWithdraw = dynamic_cast <pawsEditTextBox*> (FindWidget("ArgentsToWithdraw"));
    if(!argentsToWithdraw)
        return false;
    argentsToWithdraw->SetText("0");
    coppersToWithdraw = dynamic_cast <pawsEditTextBox*> (FindWidget("CoppersToWithdraw"));
    if(!coppersToWithdraw)
        return false;
    coppersToWithdraw->SetText("0");
    bitcentsToDeposit = dynamic_cast <pawsEditTextBox*> (FindWidget("BITCentsToDeposit"));
    if(!bitcentsToDeposit)
        return false;
    bitcentsToDeposit->SetText("0");
    denariusToDeposit = dynamic_cast <pawsEditTextBox*> (FindWidget("DenariusToDeposit"));
    if(!denariusToDeposit)
        return false;
    denariusToDeposit->SetText("0");
    argentsToDeposit = dynamic_cast <pawsEditTextBox*> (FindWidget("ArgentsToDeposit"));
    if(!argentsToDeposit)
        return false;
    argentsToDeposit->SetText("0");
    coppersToDeposit = dynamic_cast <pawsEditTextBox*> (FindWidget("CoppersToDeposit"));
    if(!coppersToDeposit)
        return false;
    coppersToDeposit->SetText("0");
    bitcentsCanExchange = dynamic_cast <pawsTextBox*> (FindWidget("BITCentsCanExchange"));
    if(!bitcentsCanExchange)
        return false;
    denariusCanExchange = dynamic_cast <pawsTextBox*> (FindWidget("DenariusCanExchange"));
    if(!denariusCanExchange)
        return false;
    argentsCanExchange = dynamic_cast <pawsTextBox*> (FindWidget("ArgentsCanExchange"));
    if(!argentsCanExchange)
        return false;
    coppersCanExchange = dynamic_cast <pawsTextBox*> (FindWidget("CoppersCanExchange"));
    if(!coppersCanExchange)
        return false;
    coinsToExchange = dynamic_cast <pawsEditTextBox*> (FindWidget("CoinsToExchange"));
    if(!coinsToExchange)
        return false;
    coinsToExchange->SetText("0");
  
    bitcents = (pawsRadioButton*)(FindWidget("BITCent"));
    denarius = (pawsRadioButton*)(FindWidget("Denarius"));
    argents = (pawsRadioButton*)(FindWidget("Argents"));
    coppers = (pawsRadioButton*)(FindWidget("Coppers"));

    feeInfo = dynamic_cast <pawsTextBox*> (FindWidget("MoneyExchangeInfo"));
    if(!feeInfo)
        return false;

    moneyWindow->SetAlwaysOnTop(true);
    adminWindow->SetAlwaysOnTop(true);
    moneyWindow->Show();
    Money->SetState(true);

    return true;
}

void pawsBankWindow::HandleMessage( MsgEntry* me )
{
    if(me->GetType() == MSGTYPE_BANKING)
    {
        psGUIBankingMessage incoming(me);

        switch(incoming.command)
        {
        case psGUIBankingMessage::VIEWBANK:
            {
                if(!IsVisible() && incoming.openWindow) 
                {
                    Show();
                }
                // Is there a better way to do this? Enlighten me.
                csString bitcents;
                csString denarius;
                csString argents;
                csString coppers;
                csString bitcentsBanked;
                csString denariusBanked;
                csString argentsBanked;
                csString coppersBanked;
                csString maxBITCents;
                csString maxDenarius;
                csString maxArgents;
                csString maxCoppers;
                csString fInfo;
                bitcentsCanDeposit->SetText(bitcents.AppendFmt("%i", incoming.bitcents));
                denariusCanDeposit->SetText(denarius.AppendFmt("%i", incoming.denarius));
                argentsCanDeposit->SetText(argents.AppendFmt("%i", incoming.argents));
                coppersCanDeposit->SetText(coppers.AppendFmt("%i", incoming.coppers));
                bitcentsCanWithdraw->SetText(bitcentsBanked.AppendFmt("%i", incoming.bitcentsBanked));
                denariusCanWithdraw->SetText(denariusBanked.AppendFmt("%i", incoming.denariusBanked));
                argentsCanWithdraw->SetText(argentsBanked.AppendFmt("%i", incoming.argentsBanked));
                coppersCanWithdraw->SetText(coppersBanked.AppendFmt("%i", incoming.coppersBanked));
                bitcentsCanExchange->SetText(maxBITCents.AppendFmt("%i", incoming.maxBITCents));
                denariusCanExchange->SetText(maxDenarius.AppendFmt("%i", incoming.maxDenarius));
                argentsCanExchange->SetText(maxArgents.AppendFmt("%i", incoming.maxArgents));
                coppersCanExchange->SetText(maxCoppers.AppendFmt("%i", incoming.maxCoppers));
                feeInfo->SetText(fInfo.AppendFmt("Fee charged for your account level: %.2f%%", incoming.exchangeFee));

                guild = incoming.guild;
                break;
            }
        }
    }
}

bool pawsBankWindow::OnButtonPressed(int /*mouseButton*/, int /*keyModifier*/, pawsWidget* widget)
{
    if(widget->GetID() == MONEYBUTTON)
    {
        Money->SetState(true);
        Admin->SetState(false);

        moneyWindow->Show();
        adminWindow->Hide();
    }

    if(widget->GetID() == ADMINBUTTON)
    {
        Money->SetState(false);
        Admin->SetState(true);

        moneyWindow->Hide();
        adminWindow->Show();
    }

    if(widget->GetID() == CIRCLESBUTTON)
    {
        denarius->SetState(false);
        argents->SetState(false);
        coppers->SetState(false);
    }

    if(widget->GetID() == OCTASBUTTON)
    {
        bitcents->SetState(false);
        argents->SetState(false);
        coppers->SetState(false);
    }

    if(widget->GetID() == HEXASBUTTON)
    {
        bitcents->SetState(false);
        denarius->SetState(false);
        coppers->SetState(false);
    }

    if(widget->GetID() == TRIASBUTTON)
    {
        bitcents->SetState(false);
        denarius->SetState(false);
        argents->SetState(false);
    }

    if(widget->GetID() == WITHDRAW)
    {
        // Send request to server.
        int bitcents = 0;
        sscanf(bitcentsToWithdraw->GetText(), "%d", &bitcents);

        int denarius = 0;
        sscanf(denariusToWithdraw->GetText(), "%d", &denarius);

        int argents = 0;
        sscanf(argentsToWithdraw->GetText(), "%d", &argents);

        int coppers = 0;
        sscanf(coppersToWithdraw->GetText(), "%d", &coppers);

        psGUIBankingMessage outgoing(psGUIBankingMessage::WITHDRAWFUNDS,
                                     guild, bitcents, denarius, argents, coppers);
        outgoing.SendMessage();

        // Reset to 0.
        bitcentsToWithdraw->SetText("0");
        denariusToWithdraw->SetText("0");
        argentsToWithdraw->SetText("0");
        coppersToWithdraw->SetText("0");

        return true;
    }

    if(widget->GetID() == DEPOSIT)
    {
        // Send request to server.
        int bitcents = 0;
        sscanf(bitcentsToDeposit->GetText(), "%d", &bitcents);

        int denarius = 0;
        sscanf(denariusToDeposit->GetText(), "%d", &denarius);

        int argents = 0;
        sscanf(argentsToDeposit->GetText(), "%d", &argents);

        int coppers = 0;
        sscanf(coppersToDeposit->GetText(), "%d", &coppers);


        psGUIBankingMessage outgoing(psGUIBankingMessage::DEPOSITFUNDS,
                                     guild, bitcents, denarius, argents, coppers);
        outgoing.SendMessage();

        // Reset to 0.
        bitcentsToDeposit->SetText("0");
        denariusToDeposit->SetText("0");
        argentsToDeposit->SetText("0");
        coppersToDeposit->SetText("0");

        return true;
    }

    if(widget->GetID() == EXCHANGE)
    {
        // Send request to server.
        int coins = 0;
        sscanf(coinsToExchange->GetText(), "%d", &coins);

        int coin = -1;
        if(bitcents->GetState())
        {
            coin = 0;
            bitcents->SetState(false);
        }
        else if(denarius->GetState())
        {
            coin = 1;
            denarius->SetState(false);
        }
        else if(argents->GetState())
        {
            coin = 2;
            argents->SetState(false);
        }
        else if(coppers->GetState())
        {
            coin = 3;
            coppers->SetState(false);
        }

        if(coin != -1)
        {
            psGUIBankingMessage outgoing(psGUIBankingMessage::EXCHANGECOINS,
                                         guild, coins, coin);
            outgoing.SendMessage();
        }

        // Reset to 0.
        coinsToExchange->SetText("0");

        return true;
    }

    return false;
}
