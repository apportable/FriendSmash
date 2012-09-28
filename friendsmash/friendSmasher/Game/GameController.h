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

#ifndef FRIENDSMASHER_GAMECONTROLLER
#define FRIENDSMASHER_GAMECONTROLLER

#include <vector>

#include "math_lib.h"

#include "Graphics.h"
#include "Sprite.h"
#include "TextureResource.h"
#include "ConfettiEffect.h"

#import "ViewController.h"
#import "AppDelegate.h"

#import <Accounts/Accounts.h>

namespace FriendSmasher
{
    namespace Game
    { 
        class GameController
        {
            
        public:

            GameController(ViewController* vc, float fScaleFactor);
            virtual ~GameController();

            void OnEnter();
            void OnExit();
            void OnUpdate();
            void OnRender();
            
            void BeginTouch(int nIndex, float fX, float fY);
            void ContinueTouch(int nIndex, float fX, float fY);
            void EndTouch(int nIndex, float fX, float fY);
                


            
            
        protected:    
            
            void SpawnEntity();
            void UpdateView(bool bSocial);
            void EndGame();
            void StartGame(bool bSocial, bool bChallenge, NSString *challengeFriendName,  NSString *challengeFriendId);
            
            System::Sprite* m_pBackgroundSprite;
            System::Sprite* m_pLoginButtonSprite;
            System::Sprite* m_pWelcomePanel;
            System::Sprite* m_pPlayButtonSprite;
            System::Sprite* m_pLogoutButtonSprite;
            System::Sprite* m_pChallengeButtonSprite;
            System::Sprite* m_pBragButtonSprite;
            System::Sprite* m_pUserImageSprite;
            System::Sprite* m_pFriendImageSprite;
            System::Sprite* m_pLoadingSprite;
            System::Sprite* m_pLoadingSpinner;
            System::Sprite* m_pLogoSprite;
            System::Sprite* m_pHeartSprite[3];
            
                
            //////
            
            System::TextureResource* m_pUserTexture;
            System::TextureResource* m_pFriendTexture;
            System::TextureResource* m_pNonFriendTexture[10];
            System::TextureResource* m_pRiserTextures[3];
    
            ///////
            
            ConfettiEffect* m_confettiEffect;
            
            ///////
            
            UILabel *m_labelName;
            UILabel *m_labelNameStatus;
            UILabel *m_labelFriendName;
            UILabel *m_labelScore;
            NSString* m_nsstrUserName;
            
            NSArray* fetchedFriendData;
            
            struct EntityInstance
            {
                System::Sprite* pSprite;
                Math::vec2 vVelocity;
                float fRotationVelocity;
                bool bFriend;
            };
            
            std::vector<EntityInstance*> m_vecEntities;
            
            int m_nEntitiesSpawned;
            
            int m_nNoSocialFriendCeleb;
            
            float m_fSpawnTimer;

            std::vector<System::Sprite*> m_vecRisers;
            
            ///////
                
            
            enum eGameState
            {
                kGAMESTATE_FRONTSCREEN_LOGGEDOUT,
                kGAMESTATE_FRONTSCREEN_LOGGEDIN_PREPARING,
                kGAMESTATE_FRONTSCREEN_LOGGEDIN_READY,
                kGAMESTATE_FRONTSCREEN_LOGGEDIN_LOADING,
                kGAMESTATE_FRONTSCREEN_NOSOCIAL_LOADING,
                kGAMESTATE_FRONTSCREEN_NOSOCIAL_READY,
                kGAMESTATE_PLAYING,
                kGAMESTATE_PLAYING_NOSOCIAL,
                kGAMESTATE_PLAYING_GAMEOVER,
                kGAMESTATE_PLAYING_GAMEOVER_NOSOCIAL,
            };
            
            
            enum eGameAchievements
            {
                kACHIEVEMENT_SCORE50 = 0,
                kACHIEVEMENT_SCORE100,
                kACHIEVEMENT_SCORE150,
                kACHIEVEMENT_SCORE200,
                kACHIEVEMENT_SCOREx3
            };
            
            
            eGameState m_kGameState;
            float m_fDeltaTime;
            
            static const int kMaxNumTouches = 5;
            Math::vec2 m_vBeginTouchPos[kMaxNumTouches];
            Math::vec2 m_vCurrentTouchPos[kMaxNumTouches];
            Math::vec2 m_vEndTouchPos[kMaxNumTouches];
            bool m_bTouched[kMaxNumTouches];
            bool m_bTouchedLastFrame[kMaxNumTouches];
            
            float m_fScaleFactor;
            
            u32 m_uLivesRemaining;
            u32 m_uCurrentScore;
            
            enum eLossType
            {
                kUNDEFINED,
                kLOSSTYPE_RANOUTOFLIVES,
                kLOSSTYPE_HITCELEB,
            };
            
            u64 m_uPlayerFBID;
            u64 m_uFriendFBID;
            
            eLossType m_kLossType;
            
            EntityInstance* m_pCelebLossEntity;
            
            ViewController* m_viewController;
            ACAccount* m_facebookAccount;
            
            
            // Facebook Integration
            
        public:
            
            void FB_CreateNewSession();
            void FB_Login();
            void FB_Logout();
            void FB_ProcessIncomingURL();
            void FB_ProcessIncomingRequest(NSString* urlString);
            void FB_ProcessIncomingFeed(NSString* urlString);
            void FB_Customize();
            void FB_SendRequest(const int nScore);
            void FB_SendFilteredRequest(const int nScore);
            void FB_SendBrag(const int nScore);
            void FB_SendScore(const int nScore);
            void FB_SendAchievement(eGameAchievements achievement);
            void FB_SendOG();
            void FB_RequestWritePermissions();
        };
    }
}

#endif