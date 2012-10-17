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

#include "debug.h"
#include "math_lib.h"
#include "random.h"
#include <stdlib.h>
#include <math.h>

namespace FriendSmasher
{
    namespace Game
    {
        GameController::GameController(ViewController* vc, float fScaleFactor) :
            m_viewController(vc),
            m_fDeltaTime(0.f),
            m_kGameState(kGAMESTATE_FRONTSCREEN_LOGGEDOUT),
            m_fScaleFactor(fScaleFactor),
            m_pBackgroundSprite(NULL),
            m_pLoginButtonSprite(NULL),
            m_pWelcomePanel(NULL),
            m_pPlayButtonSprite(NULL),
            m_pChallengeButtonSprite(NULL),
            m_pBragButtonSprite(NULL),
            m_pUserImageSprite(NULL),
            m_pFriendImageSprite(NULL),
            m_pLoadingSprite(NULL),
            m_pLoadingSpinner(NULL),
            m_pLogoSprite(NULL),
            m_pFriendTexture(NULL),
            m_nEntitiesSpawned(0),
            m_fSpawnTimer(0.f),
            m_uLivesRemaining(0),
            m_uCurrentScore(0),
            m_pUserTexture(NULL),
            m_kLossType(kUNDEFINED),
            m_pCelebLossEntity(NULL),
            m_pLogoutButtonSprite(NULL),
            m_confettiEffect(NULL),
            m_uPlayerFBID(0),
            m_nNoSocialFriendCeleb(0),
            m_uFriendFBID(0)
        {
            for (int i=0; i<kMaxNumTouches; ++i) {
                m_bTouchedLastFrame[i] = false;
                m_bTouched[i] = false;
            }
            
            for (int i=0; i<3; ++i) {
                m_pRiserTextures[i] = NULL;
            }
            
            for (int i=0; i<10; ++i) {
                m_pNonFriendTexture[i] = NULL;
            }
                       
            for (u32 i=0; i<3; ++i) {
                m_pHeartSprite[i] = NULL;
            }
            
            for (u32 i=0; i<kACHIEVEMENT_MAX; ++i) {
                m_bShouldSendAchievement[i] = false;
            }
               
            m_labelName = [[UILabel alloc] initWithFrame:CGRectMake(88.0, 36.0, 220.0, 100.0)];
            m_labelName.textAlignment = UITextAlignmentLeft;
            m_labelName.textColor = [UIColor colorWithRed:0.14 green:0.14 blue:0.14 alpha:1.0];
            m_labelName.backgroundColor = [UIColor clearColor];
            m_labelName.font = [UIFont fontWithName:@"Avenir Next Condensed" size:(28.0)];
            [vc.view addSubview: m_labelName];
            m_labelName.text = [NSString stringWithFormat:@"Welcome, Player"];
            m_labelName.hidden = NO;
            
            m_labelNameStatus = [[UILabel alloc] initWithFrame:CGRectMake(90.0, 56.0, 220.0, 100.0)];
            m_labelNameStatus.textAlignment = UITextAlignmentLeft;
            m_labelNameStatus.textColor = [UIColor colorWithRed:0.14 green:0.14 blue:0.14 alpha:1.0];
            m_labelNameStatus.backgroundColor = [UIColor clearColor];
            m_labelNameStatus.font = [UIFont fontWithName:@"Avenir Next Condensed" size:(15.0)];
            [vc.view addSubview: m_labelNameStatus];
            m_labelNameStatus.text = [NSString stringWithFormat:@"Let's smash some friends!"];
            m_labelNameStatus.hidden = YES;
            
            m_labelFriendName = [[UILabel alloc] initWithFrame:CGRectMake(6.0, 0.0, 640.0, 34.0)];
            m_labelFriendName.textAlignment = UITextAlignmentLeft;
            m_labelFriendName.textColor = [UIColor colorWithRed:0.89 green:0.29 blue:0.14 alpha:1.0];
            m_labelFriendName.backgroundColor = [UIColor clearColor];
            m_labelFriendName.font = [UIFont fontWithName:@"Avenir Next Condensed" size:(28.0)];
            [vc.view addSubview: m_labelFriendName];
            m_labelFriendName.text = [NSString stringWithFormat:@""];
            m_labelFriendName.hidden = YES;
            m_labelFriendName.shadowColor = [UIColor blackColor];
            m_labelFriendName.shadowOffset = CGSizeMake(0,1);
            
            m_labelScore = [[UILabel alloc] initWithFrame:CGRectMake(214.0, 6.0, 100.0, 20.0)];
            m_labelScore.textAlignment = UITextAlignmentRight;
            m_labelScore.textColor = [UIColor whiteColor];
            m_labelScore.backgroundColor = [UIColor clearColor];
            m_labelScore.font = [UIFont fontWithName:@"Avenir Next Condensed" size:(20.0)];
            [vc.view addSubview: m_labelScore];
            m_labelScore.text = [NSString stringWithFormat:@"Score: 0"];
            m_labelScore.hidden = YES;
            m_labelScore.shadowColor = [UIColor blackColor];
            m_labelScore.shadowOffset = CGSizeMake(0,1);
            
        }

        GameController::~GameController()
        {
            delete m_pBackgroundSprite;
            delete m_pLoginButtonSprite;
            delete m_pPlayButtonSprite;
            delete m_pUserImageSprite;
            delete m_pFriendImageSprite;
            delete m_pLoadingSprite;
            delete m_pLoadingSpinner;
            delete m_pLogoSprite;
            delete m_pFriendTexture;
            delete m_pUserTexture;
            delete m_pChallengeButtonSprite;
            delete m_pBragButtonSprite;
            delete m_pWelcomePanel;
            delete m_confettiEffect;
          
            for (int i=0; i<3; ++i) {
                delete m_pRiserTextures[i];
            }
            
            for (int i=0; i<10; ++i) {
                delete m_pNonFriendTexture[i];
            }

            for (u32 i=0; i<3; ++i) {
                delete m_pHeartSprite[i];
            }
        }
		
        void GameController::OnEnter()
        {
            Random::Seed();
            
            System::Graphics::Instance()->Initialise(m_fScaleFactor);
            
            System::TextureResource* pBackgroundTextureResource = new System::TextureResource();
            pBackgroundTextureResource->CreateFromFile("Art/frontscreen_background.png");
            m_pBackgroundSprite = new System::Sprite(pBackgroundTextureResource);
            
            System::TextureResource* pLogoTextureResource = new System::TextureResource();
            pLogoTextureResource->CreateFromFile("Art/logo.png");
            m_pLogoSprite = new System::Sprite(pLogoTextureResource);
            m_pLogoSprite->SetPosition(Math::vec2(0.f, 0.f));
            m_pLogoSprite->SetLayer(10);
            
            
            System::TextureResource* pWelcomePanelTextureResource = new System::TextureResource();
            pWelcomePanelTextureResource->CreateFromFile("Art/welcome_panel.png");
            m_pWelcomePanel = new System::Sprite(pWelcomePanelTextureResource);
            m_pWelcomePanel->SetPosition(Math::vec2(0.f, 132.f));
            m_pWelcomePanel->SetLayer(10);
            
            
            System::TextureResource* pLoginButtonTextureResource = new System::TextureResource();
            pLoginButtonTextureResource->CreateFromFile("Art/login_button.png");
            m_pLoginButtonSprite = new System::Sprite(pLoginButtonTextureResource);
            m_pLoginButtonSprite->SetPosition(Math::vec2(110.f, 540.f));
            m_pLoginButtonSprite->SetLayer(10);
            
            System::TextureResource* pLogoutButtonTextureResource = new System::TextureResource();
            pLogoutButtonTextureResource->CreateFromFile("Art/logout_button.png");
            m_pLogoutButtonSprite = new System::Sprite(pLogoutButtonTextureResource);
            m_pLogoutButtonSprite->SetPosition(Math::vec2(288.f, 862.f));
            m_pLogoutButtonSprite->SetLayer(10);
            m_pLogoutButtonSprite->SetDraw(false);
            
            
            System::TextureResource* pPlayButtonTextureResource = new System::TextureResource();
            pPlayButtonTextureResource->CreateFromFile("Art/playnow_button.png");
            m_pPlayButtonSprite = new System::Sprite(pPlayButtonTextureResource);
            m_pPlayButtonSprite->SetDraw(false);
            m_pPlayButtonSprite->SetLayer(10);
            m_pPlayButtonSprite->SetPosition(Math::vec2(252.f, 312.f));
            
            System::TextureResource* pChallengeButtonTextureResource = new System::TextureResource();
            pChallengeButtonTextureResource->CreateFromFile("Art/challenge_button.png");
            m_pChallengeButtonSprite = new System::Sprite(pChallengeButtonTextureResource);
            m_pChallengeButtonSprite->SetDraw(false);
            m_pChallengeButtonSprite->SetLayer(10);
            m_pChallengeButtonSprite->SetPosition(Math::vec2(154.f, 614.f));
            
            
            System::TextureResource* pBragButtonTextureResource = new System::TextureResource();
            pBragButtonTextureResource->CreateFromFile("Art/brag_button.png");
            m_pBragButtonSprite = new System::Sprite(pBragButtonTextureResource);
            m_pBragButtonSprite->SetDraw(false);
            m_pBragButtonSprite->SetLayer(10);
            m_pBragButtonSprite->SetPosition(Math::vec2(0.f, 738.f));
            
            
            System::TextureResource* pLoadingTextureResource = new System::TextureResource();
            pLoadingTextureResource->CreateFromFile("Art/isloading.png");
            m_pLoadingSprite = new System::Sprite(pLoadingTextureResource);
            m_pLoadingSprite->SetDraw(false);
            m_pLoadingSprite->SetLayer(10);
            m_pLoadingSprite->SetPosition(Math::vec2(210.f, 460.f));
            
            System::TextureResource* pLoadingSpinnerTextureResource = new System::TextureResource();
            pLoadingSpinnerTextureResource->CreateFromFile("Art/loadingspinner.tga");
            m_pLoadingSpinner = new System::Sprite(pLoadingSpinnerTextureResource);
            m_pLoadingSpinner->SetDraw(false);
            m_pLoadingSpinner->SetLayer(10);
            m_pLoadingSpinner->SetPivot(Math::vec2(m_pLoadingSpinner->GetWidth()*0.5f, m_pLoadingSpinner->GetHeight()*0.5f));
            m_pLoadingSpinner->SetPosition(Math::vec2(274.f, 504.f));
            
            
            System::TextureResource* pHeartTextureResource = new System::TextureResource();
            pHeartTextureResource->CreateFromFile("Art/heart_red.tga");
            
            for (u32 i=0; i<3; ++i)
            {
                m_pHeartSprite[i] = new System::Sprite(pHeartTextureResource);
                m_pHeartSprite[i]->SetDraw(false);
                m_pHeartSprite[i]->SetLayer(20 + i);
                m_pHeartSprite[i]->SetPosition(Math::vec2(10.f + (72.f * static_cast<float>(i)), 60.f));
            }
                 
            for (int i=0; i<10; ++i)
            {
                m_pNonFriendTexture[i] = new System::TextureResource();
                char buf[128];
                sprintf(buf, "nonfriend_%d.png", i+1);
                m_pNonFriendTexture[i]->CreateFromFile(buf);
            }
                        
            m_pRiserTextures[0] = new System::TextureResource();
            m_pRiserTextures[0]->CreateFromFile("Art/plus_1.tga");
            
            m_pRiserTextures[1] = new System::TextureResource();
            m_pRiserTextures[1]->CreateFromFile("Art/plus_2.tga");
            
            m_pRiserTextures[2] = new System::TextureResource();
            m_pRiserTextures[2]->CreateFromFile("Art/plus_3.tga");
                        
            m_confettiEffect = ConfettiEffect::Create( 256 );
			m_confettiEffect->SetMaxParticlesPerFrame( 256 );
			m_confettiEffect->SetMinPosition( Math::vec3( 0.0f, 0.0f, -900.0f ) );
			m_confettiEffect->SetMaxPosition( Math::vec3( 0.0f, 0.0f, -900.0f ) );
			m_confettiEffect->SetMinStartVelocity( Math::vec3( -400.0f, -400.0f, -80.0f ) );
			m_confettiEffect->SetMaxStartVelocity( Math::vec3( 400.0f,  400.0f,  -80.0f ) );
			m_confettiEffect->SetFaceCamera( false );
            m_confettiEffect->SetMinStartScale( 4.0f );
            m_confettiEffect->SetMaxStartScale( 12.0f );
            m_confettiEffect->SetMinDamping( 0.95f );
            m_confettiEffect->SetMaxDamping( 0.99f );
            m_confettiEffect->SetMinRotationDelta(  1.0f );
            m_confettiEffect->SetMaxRotationDelta( 10.0f );
            m_confettiEffect->SetGravity( Math::vec3( 0.0f, 0.f, 40.0f ) );
            
            static Math::vec3 colours[] =
            {
                Math::vec3( 1.0f, 0.2f,  0.0f ),
                Math::vec3( 1.0f, 0.5f,  0.0f ),
                Math::vec3( 1.0f, 0.7f,  0.0f ),
                Math::vec3( 1.0f, 0.3f,  0.0f ),
                Math::vec3( 0.0f, 0.0f,  0.0f ),
            };
            
            unsigned int num_colours = sizeof(colours) / sizeof(colours[0]);
            
            m_confettiEffect->SetColours( colours, num_colours );
            
            
            FB_CreateNewSession();
            
#ifdef NO_FACEBOOK_INTEGRATION
            if (m_kGameState == kGAMESTATE_FRONTSCREEN_NOSOCIAL_READY) {
                UpdateView(false);
            }
#else
            if ([FBSession activeSession].state == FBSessionStateCreatedTokenLoaded) {
                FB_Login();
                
            }
            else if (m_kGameState == kGAMESTATE_FRONTSCREEN_NOSOCIAL_READY) {
                UpdateView(false);
            }
#endif
 
        }

        void GameController::OnExit()
        {
            System::Graphics::Instance()->Uninitialise();
        }
		
        void GameController::OnRender()
        {
            System::Graphics::Instance()->Update();
            
            static float fov		= 45.0f;
            static float fAspect	= 0.67f;
            static float fNearClip  = 1.0f;
            static float fFarClip	= 1000.0f;
            
            Math::mtx44 viewMat;
            Math::mtx44 projMat;
            
            mtx44MakeIdentity(viewMat);
            
            mtx44MakePerspectiveRH( projMat, fov, fAspect, fNearClip, fFarClip );
            
			m_confettiEffect->Render(viewMat, projMat);
            
        }
        
        void GameController::OnUpdate()
        {   
            if (m_kGameState == kGAMESTATE_FRONTSCREEN_LOGGEDOUT)
            {
                if (!m_bTouched[0] && m_bTouchedLastFrame[0]) {
                                    
                    if (m_pLoginButtonSprite->IsPointInside(m_vEndTouchPos[0], 25.f)) 
                    {
#ifndef NO_FACEBOOK_INTEGRATION
                        if (![FBSession activeSession].isOpen) {
                            
                            FB_CreateNewSession();
                            FB_Login();
                        }
#endif
                    }
                } 
            }
            else if (m_kGameState == kGAMESTATE_FRONTSCREEN_LOGGEDIN_PREPARING)
            {
                if (m_pUserTexture && m_pUserTexture->GetIsReady())
                {
                    m_pUserImageSprite = new System::Sprite(m_pUserTexture);
                    m_pUserImageSprite->SetLayer(10);
                    m_pUserImageSprite->SetPivot(Math::vec2(m_pUserImageSprite->GetWidth()*0.5f, m_pUserImageSprite->GetHeight()*0.5f));
                    m_pUserImageSprite->SetPosition(Math::vec2(85.f, 217.f));
                    m_pUserImageSprite->SetScale(Math::vec2(0.58f, 0.58f));
                    
                    m_labelName.text = [NSString stringWithFormat:@"Welcome, %@", m_nsstrUserName];
                    
                    m_labelNameStatus.hidden = NO;
                    
                    m_pPlayButtonSprite->SetDraw(true);
                    m_pWelcomePanel->SetDraw(true);
                    m_pLogoutButtonSprite->SetDraw(true);
                                        
                    m_kGameState = kGAMESTATE_FRONTSCREEN_LOGGEDIN_READY;
                    
                }
            }
            else if (m_kGameState == kGAMESTATE_FRONTSCREEN_LOGGEDIN_READY || m_kGameState == kGAMESTATE_FRONTSCREEN_NOSOCIAL_READY)
            {
                if (!m_bTouched[0] && m_bTouchedLastFrame[0] && m_pPlayButtonSprite->IsPointInside(m_vEndTouchPos[0], 0.f)
                    && m_pPlayButtonSprite->GetDraw()) 
                {
                    StartGame(m_kGameState != kGAMESTATE_FRONTSCREEN_NOSOCIAL_READY, false, nil, nil);
                }
                
                if (!m_bTouched[0] && m_bTouchedLastFrame[0] && m_pLogoutButtonSprite->IsPointInside(m_vEndTouchPos[0], 0.f)
                    && m_pLogoutButtonSprite->GetDraw()) 
                {
                    FB_Logout();
                }
                
                if (!m_bTouched[0] && m_bTouchedLastFrame[0] && m_pBragButtonSprite->IsPointInside(m_vEndTouchPos[0], 0.f)
                    && m_pBragButtonSprite->GetDraw()) 
                {
                    FB_SendBrag(m_uCurrentScore);
                }
                
                if (!m_bTouched[0] && m_bTouchedLastFrame[0] && m_pChallengeButtonSprite->IsPointInside(m_vEndTouchPos[0], 0.f)
                    && m_pChallengeButtonSprite->GetDraw()) 
                {
                    FB_SendRequest(m_uCurrentScore);
                }
            }
          
            else if (m_kGameState == kGAMESTATE_FRONTSCREEN_NOSOCIAL_LOADING || m_kGameState == kGAMESTATE_FRONTSCREEN_LOGGEDIN_LOADING)
            {
                m_pLoadingSpinner->SetRotation(m_pLoadingSpinner->GetRotation() + 0.25f);
                
                if (m_pFriendTexture && m_pFriendTexture->GetIsReady())
                {
                    m_pLoadingSprite->SetDraw(false);
                    m_pLoadingSpinner->SetDraw(false);
                    m_pLogoSprite->SetDraw(false);
                    
                    m_uCurrentScore = 0;
                    m_uLivesRemaining = 3;
                    
                    for (u32 i=0; i<m_uLivesRemaining; ++i) {
                        m_pHeartSprite[i]->SetDraw(true);
                    }
                    
                    m_labelScore.text = [NSString stringWithFormat:@"Score: %u", m_uCurrentScore];
                    m_labelScore.hidden = NO;
                    
                    m_labelFriendName.hidden = NO;

                    m_kLossType = kUNDEFINED;
                    m_pCelebLossEntity = NULL;
                    
                    if (m_kGameState == kGAMESTATE_FRONTSCREEN_NOSOCIAL_LOADING) {
                        m_kGameState = kGAMESTATE_PLAYING_NOSOCIAL;  
                    }
                    else {
                        m_kGameState = kGAMESTATE_PLAYING;                        
                    }

                    return;
                }
   
            }
            else if (m_kGameState == kGAMESTATE_PLAYING || m_kGameState == kGAMESTATE_PLAYING_NOSOCIAL)
            {
                m_fSpawnTimer -= 1.0f / 30.0f;
                
                if (m_fSpawnTimer < 0.f) 
                {
                    SpawnEntity();
                    m_fSpawnTimer = 0.7;
                }
                               
                int nFrameScore = 0;
                
                std::vector<EntityInstance*>::iterator itr = m_vecEntities.begin();
                std::vector<EntityInstance*>::iterator end = m_vecEntities.end();
                for (; itr != end; ++itr)
                {
                    EntityInstance* pCurrentEntity = *itr;
                    pCurrentEntity->pSprite->SetPosition(pCurrentEntity->pSprite->GetPosition() + pCurrentEntity->vVelocity);
                    pCurrentEntity->vVelocity.y += 1.f;
                    pCurrentEntity->pSprite->SetRotation(pCurrentEntity->pSprite->GetRotation() + pCurrentEntity->fRotationVelocity);
                    
                    if (pCurrentEntity->pSprite->GetPosition().y > 1000.f && pCurrentEntity->bFriend) {
                        m_vecEntities.erase(itr);
                        pCurrentEntity->pSprite->SetDraw(false);
                        delete pCurrentEntity->pSprite;
                        delete pCurrentEntity;
                        m_uLivesRemaining--;
                        m_pHeartSprite[m_uLivesRemaining]->SetDraw(false);
                        m_kLossType = kLOSSTYPE_RANOUTOFLIVES;

                        itr = m_vecEntities.begin();
                        end = m_vecEntities.end();
                        continue;
                    }
                    
                    if (m_bTouched[0] && !m_bTouchedLastFrame[0] && pCurrentEntity->pSprite->IsPointInside(m_vBeginTouchPos[0], 25.f)) 
                    {
                        if (!pCurrentEntity->bFriend)
                        {
                            m_pCelebLossEntity = pCurrentEntity;
                            m_pCelebLossEntity->pSprite->SetLayer(m_nEntitiesSpawned + 100);
                            m_kLossType = kLOSSTYPE_HITCELEB;
                            m_uLivesRemaining = 0;
                            break;
                        }
                        else
                        {
                            nFrameScore++;
                            
                            m_vecEntities.erase(itr);
                            pCurrentEntity->pSprite->SetDraw(false);
                            delete pCurrentEntity->pSprite;
                            delete pCurrentEntity;
                            
                            // If they start to score well, spawn extras
                            if ( !(m_uCurrentScore % 10) )
                            {
                                for (int i=0; i<(m_uCurrentScore/20); ++i) {
                                    SpawnEntity();
                                }
                            }
                            
                            itr = m_vecEntities.begin();
                            end = m_vecEntities.end();
                            continue;
                        }
                    }
                }
                
                if (nFrameScore > 0)
                {
                    System::Sprite* pRiser = new System::Sprite(m_pRiserTextures[nFrameScore < 3 ? (nFrameScore-1) : 2]);
                    
                    pRiser->SetPivot(Math::vec2(pRiser->GetWidth() * 0.5f, pRiser->GetHeight() * 0.5f));
                    pRiser->SetPosition(m_vBeginTouchPos[0]);
                    pRiser->SetLayer(1000);
                    m_vecRisers.push_back(pRiser);
                    
                    if (nFrameScore > 1) {
                        m_confettiEffect->SetMaxPosition(Math::vec3(m_vBeginTouchPos[0].x - 320.f, (m_vBeginTouchPos[0].y - 480.f) * -1.f, -900.f));
                        m_confettiEffect->SetMinPosition(Math::vec3(m_vBeginTouchPos[0].x - 320.f, (m_vBeginTouchPos[0].y - 480.f) * -1.f, -900.f));
                        m_confettiEffect->SpawnParticles(80);
                    
                        if (nFrameScore >= 3) {
                            m_bShouldSendAchievement[kACHIEVEMENT_SCOREx3] = true;
                            
                        }
                    }
                    
                    u32 uOldScore = m_uCurrentScore;
                    m_uCurrentScore += (nFrameScore * nFrameScore);
                    
                    if (uOldScore < 50 && m_uCurrentScore >= 50) {
                        m_bShouldSendAchievement[kACHIEVEMENT_SCORE50] = true;
                    }
                    else if (uOldScore < 100 && m_uCurrentScore >= 100) {
                        m_bShouldSendAchievement[kACHIEVEMENT_SCORE100] = true;
                    }
                    else if (uOldScore < 150 && m_uCurrentScore >= 150) {
                        m_bShouldSendAchievement[kACHIEVEMENT_SCORE150] = true;
                    }
                    else if (uOldScore < 200 && m_uCurrentScore >= 200) {
                        m_bShouldSendAchievement[kACHIEVEMENT_SCORE200] = true;
                    }
                    
                    
                    m_labelScore.text = [NSString stringWithFormat:@"Score: %u", m_uCurrentScore];
                    
                }
                
                m_confettiEffect->Update();
                
                std::vector<System::Sprite*>::iterator riseritr = m_vecRisers.begin();
                std::vector<System::Sprite*>::iterator riserend = m_vecRisers.end();
                for (; riseritr != riserend; ++riseritr)
                {
                    System::Sprite* pCurrentRiser = *riseritr;
                    pCurrentRiser->SetPosition(Math::vec2(pCurrentRiser->GetPosition().x, pCurrentRiser->GetPosition().y - 2.f));
                    pCurrentRiser->SetAlpha(pCurrentRiser->GetAlpha() - 0.025f);
                    
                    if (pCurrentRiser->GetAlpha() <= 0.f) {
                        pCurrentRiser->SetDraw(false);
                        m_vecRisers.erase(riseritr);
                        delete pCurrentRiser;
                        return;
                    }
                }
                
                // Game over
                if (m_uLivesRemaining == 0) {
                    
                    std::vector<System::Sprite*>::iterator riseritr = m_vecRisers.begin();
                    std::vector<System::Sprite*>::iterator riserend = m_vecRisers.end();
                    for (; riseritr != riserend; ++riseritr)
                    {
                        System::Sprite* pCurrentRiser = *riseritr;
                        pCurrentRiser->SetDraw(false);
                        delete pCurrentRiser;
                    }
                    m_vecRisers.clear();
                    
                    m_confettiEffect->Reset();
                    
                    m_kGameState = m_kGameState == kGAMESTATE_PLAYING ? kGAMESTATE_PLAYING_GAMEOVER : kGAMESTATE_PLAYING_GAMEOVER_NOSOCIAL;
                }
                            
            }
            else if (m_kGameState == kGAMESTATE_PLAYING_GAMEOVER || m_kGameState == kGAMESTATE_PLAYING_GAMEOVER_NOSOCIAL)
            {
                if (m_kLossType == kLOSSTYPE_HITCELEB && m_pCelebLossEntity)
                {
                    float fDistanceToUpright = fmodf(m_pCelebLossEntity->pSprite->GetRotation(),6.28318531f);
                    
                    m_pCelebLossEntity->pSprite->SetRotation(m_pCelebLossEntity->pSprite->GetRotation() + (fDistanceToUpright * 0.05f));

                    m_pCelebLossEntity->pSprite->SetScale(m_pCelebLossEntity->pSprite->GetScale() * Math::vec2(1.1f, 1.1f));
                    
                    if (m_pCelebLossEntity->pSprite->GetScale().x > 30.f)
                    {
                        EndGame();
                    }
                }
                else {
                    EndGame();
                }
                
            }
            
            for (int i=0; i<kMaxNumTouches; ++i) {
				m_bTouchedLastFrame[i] = m_bTouched[i];
            }
        }
        
        void GameController::StartGame(bool bSocial, bool bChallenge, NSString *challengeFriendName,  NSString *challengeFriendId)
        {
            m_pLoadingSprite->SetDraw(true);
            m_pLoadingSpinner->SetDraw(true);
            
            m_pPlayButtonSprite->SetDraw(false);
            m_pWelcomePanel->SetDraw(false);
            m_pLogoutButtonSprite->SetDraw(false);
            m_pChallengeButtonSprite->SetDraw(false);
            m_pBragButtonSprite->SetDraw(false);
            
            for (u32 i=0; i<kACHIEVEMENT_MAX; ++i) {
                m_bShouldSendAchievement[i] = false;
            }
            
            m_labelName.hidden = YES;
            m_labelNameStatus.hidden = YES;
            
            if (bSocial)
            {
                m_pUserImageSprite->SetDraw(false);   
                
                if (bChallenge) {
                    
                    m_labelFriendName.text = [NSString stringWithFormat:@"Smash %@!", [[challengeFriendName componentsSeparatedByString:@" "] objectAtIndex:0]];
                    
                    if (m_pFriendTexture) { 
                        delete m_pFriendTexture;
                    }
                    
                    m_uFriendFBID = [challengeFriendId intValue];
                    
                    m_pFriendTexture = new System::TextureResource();
                    m_pFriendTexture->CreateFromFBID(m_uFriendFBID, 128, 128);
                    
                    m_kGameState = kGAMESTATE_FRONTSCREEN_LOGGEDIN_LOADING;
                    
                }
                else {
                    
                    m_kGameState = kGAMESTATE_FRONTSCREEN_LOGGEDIN_LOADING;
#ifndef NO_FACEBOOK_INTEGRATION
                    [FBRequestConnection startForMyFriendsWithCompletionHandler:^(FBRequestConnection *connection, id result, NSError *error) {
                      
                      if (!error && result)
                      {
                        fetchedFriendData = [[NSArray alloc] initWithArray:[result objectForKey:@"data"]];
                      
                        NSDictionary *friendData = [fetchedFriendData objectAtIndex: arc4random() % fetchedFriendData.count];
                      
                        NSString *friendId = [friendData objectForKey:@"id"];
                        NSString *friendName = [friendData objectForKey:@"name"];
                      
                        m_labelFriendName.text = [NSString stringWithFormat:@"Smash %@!", [[friendName componentsSeparatedByString:@" "] objectAtIndex:0]];
                      
                        if (m_pFriendTexture) {
                            delete m_pFriendTexture;
                        }
                      
                        m_uFriendFBID = [friendId intValue];
                      
                        m_pFriendTexture = new System::TextureResource();
                        m_pFriendTexture->CreateFromFBID(m_uFriendFBID, 128, 128);
                      
                      }
                      
                    }];
                    
#endif
                }
                
                
            }
            else
            {
                if (m_pFriendTexture) {
                    delete m_pFriendTexture;
                }
                
                m_nNoSocialFriendCeleb = Random::GetRandom(1, 10);
                
                char buf[128];
                sprintf(buf, "nonfriend_%d.png", m_nNoSocialFriendCeleb);
                
                m_pFriendTexture = new System::TextureResource();
                m_pFriendTexture->CreateFromFile(buf);
                
                
                NSArray *celebArray =  [NSArray arrayWithObjects:
                                        [NSString stringWithFormat:@"Einstein"],
                                        [NSString stringWithFormat:@"Xzibit"],
                                        [NSString stringWithFormat:@"Goldsmith"],
                                        [NSString stringWithFormat:@"Sinatra"],
                                        [NSString stringWithFormat:@"George"],
                                        [NSString stringWithFormat:@"Jacko"],
                                        [NSString stringWithFormat:@"Rick"],
                                        [NSString stringWithFormat:@"Keanu"],
                                        [NSString stringWithFormat:@"Arnie"],
                                        [NSString stringWithFormat:@"Jean-Luc"],
                                        nil];
                
                m_labelFriendName.text = [NSString stringWithFormat:@"Smash %@ !", [celebArray objectAtIndex:m_nNoSocialFriendCeleb-1]];
                
                m_kGameState = kGAMESTATE_FRONTSCREEN_NOSOCIAL_LOADING;
            }
            
            
        }
        
        void GameController::EndGame()
        {
            std::vector<EntityInstance*>::iterator itr = m_vecEntities.begin();
            std::vector<EntityInstance*>::iterator end = m_vecEntities.end();
            for (; itr != end; ++itr)
            {
                EntityInstance* pCurrentEntity = *itr;
                delete pCurrentEntity->pSprite;
                delete pCurrentEntity;
            }
            
            if (m_bShouldSendAchievement[kACHIEVEMENT_SCOREx3]) {
                FB_SendAchievement(kACHIEVEMENT_SCOREx3);
            }
            
            if (m_bShouldSendAchievement[kACHIEVEMENT_SCORE200]) {
                FB_SendAchievement(kACHIEVEMENT_SCORE200);
            }
            else if (m_bShouldSendAchievement[kACHIEVEMENT_SCORE150]) {
                FB_SendAchievement(kACHIEVEMENT_SCORE150);
            }
            else if (m_bShouldSendAchievement[kACHIEVEMENT_SCORE100]) {
                FB_SendAchievement(kACHIEVEMENT_SCORE100);
            }
            else if (m_bShouldSendAchievement[kACHIEVEMENT_SCORE50]) {
                FB_SendAchievement(kACHIEVEMENT_SCORE50);     
            }
            
            m_vecEntities.clear();
            
            m_uLivesRemaining = 3;
            for (u32 i=0; i<m_uLivesRemaining; ++i) {
                m_pHeartSprite[i]->SetDraw(false);
            }
            
            m_pPlayButtonSprite->SetDraw(true);
            m_pWelcomePanel->SetDraw(true);
            m_pLogoSprite->SetDraw(true);
            
            delete m_pFriendTexture;
            m_pFriendTexture = NULL;
            
            m_labelScore.hidden = YES;
            m_labelFriendName.hidden = YES;
            
            //m_labelName.text = [NSString stringWithFormat:@"Game Over!"];
            m_labelNameStatus.text = [NSString stringWithFormat:@"You scored %d!", m_uCurrentScore];
            
            m_labelName.hidden = NO;
            m_labelNameStatus.hidden = NO;
            
            if (m_kGameState == kGAMESTATE_PLAYING_GAMEOVER)
            {
                m_pUserImageSprite->SetDraw(true);
                m_pLogoutButtonSprite->SetDraw(true);
                m_pChallengeButtonSprite->SetDraw(true);
                m_pBragButtonSprite->SetDraw(true);
                
                FB_SendScore(m_uCurrentScore);
                
                m_kGameState = kGAMESTATE_FRONTSCREEN_LOGGEDIN_READY;
            }
            else {
                m_kGameState = kGAMESTATE_FRONTSCREEN_NOSOCIAL_READY;
            }
        }
        
        
        void GameController::SpawnEntity()
        {
            EntityInstance* pEntity = new EntityInstance;
            m_vecEntities.push_back(pEntity);
            
            float fEntityType = Random::GetRandom(0.f, 100.f);

            
            if (fEntityType < 80.f) {
                pEntity->pSprite = new System::Sprite(m_pFriendTexture);
                pEntity->bFriend = true;
            }
            else {
                
                int nCelebToSpawn = Random::GetRandom(0, 9);
                
                if (m_kGameState == kGAMESTATE_PLAYING_NOSOCIAL) 
                {
                    while (nCelebToSpawn == (m_nNoSocialFriendCeleb-1))
                    {
                        nCelebToSpawn = Random::GetRandom(0, 9);    
                    }
                }
                
                pEntity->pSprite = new System::Sprite(m_pNonFriendTexture[nCelebToSpawn]);
                pEntity->bFriend = false;
            }
            
            
            pEntity->pSprite->SetLayer(10 + m_nEntitiesSpawned);
            pEntity->pSprite->SetPivot(Math::vec2(pEntity->pSprite->GetWidth()*0.5f, pEntity->pSprite->GetHeight()*0.5f));
            pEntity->pSprite->SetPosition(Math::vec2(Random::GetRandom(-200.f, 840.f), 1000.f));
            
            pEntity->fRotationVelocity = Random::GetRandom(-0.5f, 0.5f);

            float fDistanceToMiddle = Random::GetRandom(200.f, 440.f) - pEntity->pSprite->GetPosition().x;
            
            pEntity->vVelocity = Math::vec2(fDistanceToMiddle * Random::GetRandom(0.02f, 0.03f), Random::GetRandom(-42.f, -34.f));
            
            m_nEntitiesSpawned++;
        }
        

        
        void GameController::UpdateView(bool bSocial)
        {
            if (bSocial)
            {
#ifndef NO_FACEBOOK_INTEGRATION
                if ([FBSession activeSession].isOpen) {
                    m_kGameState = kGAMESTATE_FRONTSCREEN_LOGGEDIN_PREPARING;
                    
                    m_pLoginButtonSprite->SetDraw(false);
                    
                    FB_Customize();
                    
                } else {
                    m_kGameState = kGAMESTATE_FRONTSCREEN_LOGGEDOUT;
                }
#endif
            }
            else {
                m_pLoginButtonSprite->SetDraw(false);
                
                m_pPlayButtonSprite->SetDraw(true);
                m_pWelcomePanel->SetDraw(true);
                
                m_kGameState = kGAMESTATE_FRONTSCREEN_NOSOCIAL_READY;
            }
        }
          
        void GameController::BeginTouch(int nIndex, float fX, float fY)
        {
            m_bTouched[nIndex] = true;
            m_vBeginTouchPos[nIndex] = Math::vec2(fX, fY);
            m_vCurrentTouchPos[nIndex] = Math::vec2(fX, fY);
        }
        
        void GameController::ContinueTouch(int nIndex, float fX, float fY)
        {
            m_vCurrentTouchPos[nIndex] = Math::vec2(fX, fY);
        }
        
        void GameController::EndTouch(int nIndex, float fX, float fY)
        {
            m_bTouched[nIndex] = false;
            m_vEndTouchPos[nIndex] = Math::vec2(fX, fY);	
            m_vCurrentTouchPos[nIndex] = Math::vec2::allzero;
        }
    }
		
}