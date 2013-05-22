/*
 * Copyright 2012 Facebook
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "GameController.h"
#include <Social/Social.h>
#include <Social/SLComposeViewController.h>

namespace FriendSmasher
{
    namespace Game
    {            
        
        // Create a Facebook session for a given set of permissions
        
        void GameController::FB_CreateNewSession()
        {
            m_kGameState = kGAMESTATE_FRONTSCREEN_NOSOCIAL_READY;
            return;
        }
        
        // Attempt to open the session - perhaps tabbing over to Facebook to authorise
        void GameController::FB_Login()
        {
        }
        
        void GameController::FB_Customize()
        {
        }
        
        void GameController::FB_ProcessIncomingURL(NSURL* targetURL)
        {
        }
        
        void GameController::FB_ProcessIncomingRequest(NSURL* targetURL)
        {
        }
        
        void GameController::FB_ProcessIncomingFeed(NSURL* targetURL)
        {
        }
        
        void GameController::FB_RequestWritePermissions()
        {
        }
        
        void GameController::FB_SendScore(const int nScore)
        {
        }
        
        void GameController::FB_SendAchievement(eGameAchievements achievement)
        {
        }
        
        void GameController::FB_SendOG()
        {
        }
  
        void GameController::FB_SendRequest(const int nScore)
        {
        }
        
        void GameController::FB_SendFilteredRequest(const int nScore)
        {
        }
        
        void GameController::FB_SendBrag(const int nScore)
        {
        }
        
        void GameController::FB_Logout()
        {
            // Log out of Facebook and reset our session
            // Enable this code if you implement a logout feature
            //[[FBSession activeSession] closeAndClearTokenInformation];
            //[FBSession setActiveSession:nil];
            
            m_pLoginButtonSprite->SetDraw(true);
            
            m_pPlayButtonSprite->SetDraw(false);
            m_pLogoutButtonSprite->SetDraw(false);
            m_pChallengeButtonSprite->SetDraw(false);
            m_pBragButtonSprite->SetDraw(false);
            m_pUserImageSprite->SetDraw(false);
            
            m_labelName.text = [NSString stringWithFormat:@"Welcome, Player"];
            
            m_labelNameStatus.hidden = YES;
            m_labelFriendName.hidden = YES;
            m_labelScore.hidden = YES;
            
            m_kGameState = kGAMESTATE_FRONTSCREEN_LOGGEDOUT;
        }
    
    }
		
}