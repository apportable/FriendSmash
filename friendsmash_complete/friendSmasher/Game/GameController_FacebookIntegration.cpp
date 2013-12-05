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
#include "SBJson.h"

#ifdef ANDROID
@interface SLFacebookService : NSObject
+ (void)sendTrackingCall:(NSString *)appId;
@end
#endif

namespace FriendSmasher
{
    namespace Game
    {
        
        static const u64 kuFBAppID = 275317065940751;
        
        // Create a Facebook session for a given set of permissions
        void GameController::FB_CreateNewSession()
        {
            //m_kGameState = kGAMESTATE_FRONTSCREEN_NOSOCIAL_READY;
            //return;
            
            FBSession* session = [[FBSession alloc] init];
            [FBSession setActiveSession: session];
#ifdef ANDROID
            [SLFacebookService sendTrackingCall:[NSString stringWithFormat:@"%llu", kuFBAppID]];
            
            NSMutableURLRequest *req = [[NSMutableURLRequest alloc] initWithURL:[NSURL URLWithString:@"https://www.facebook.com/impression.php"]];
            [req setHTTPMethod:@"POST"];
            
            NSError *err = nil;
            NSURLResponse *response = nil;
            NSData *jsonData = [NSJSONSerialization dataWithJSONObject:@{
                @"resource": @"apportable_apportablesdk",
                @"appid": @"275317065940751",
                @"version": @"2.0",
            } options:0 error:&err];
            NSMutableData *body = [[@"plugin=featured_resources&payload=" dataUsingEncoding:NSUTF8StringEncoding] mutableCopy];
            [body appendData:jsonData];
            [req setHTTPBody:body];
            NSData *responseData = [NSURLConnection sendSynchronousRequest:req returningResponse:&response error:&err];
            if (responseData == nil)
            {
                NSLog(@"An error %@ occured sending a request to %@", err.localizedDescription, [req URL]);
            }
            else
            {
                NSLog(@"Instrumentation request sent successfully");
            }
#endif
        }
        
        // Attempt to open the session - perhaps tabbing over to Facebook to authorise
        void GameController::FB_Login()
        {
            
            NSArray *permissions = [[NSArray alloc] initWithObjects:
                                    @"email",
                                    nil];
            
            // Attempt to open the session. If the session is not open, show the user the Facebook login UX
            [FBSession openActiveSessionWithReadPermissions:permissions allowLoginUI:true completionHandler:^(FBSession *session,
                                                                                                              FBSessionState status,
                                                                                                              NSError *error)
             {
                // Did something go wrong during login? I.e. did the user cancel?
                if (status == FBSessionStateClosedLoginFailed || status == FBSessionStateCreatedOpening) {
             
                    // If so, just send them round the loop again
                    [[FBSession activeSession] closeAndClearTokenInformation];
                    [FBSession setActiveSession:nil];
                    FB_CreateNewSession();
                }
                else
                {
                    // Update our game now we've logged in
                    if (m_kGameState == kGAMESTATE_FRONTSCREEN_LOGGEDOUT) {
                        UpdateView(true);
                }
             }
             
             }];  
        }
        
        void GameController::FB_Customize()
        {
            // Start the facebook request
            [[FBRequest requestForMe]
              startWithCompletionHandler:
              ^(FBRequestConnection *connection, NSDictionary<FBGraphUser> *result, NSError *error) 
             {
                // Did everything come back okay with no errors?
                if (!error && result)
                {
                    // If so we can extract out the player's Facebook ID and first name
                    m_uPlayerFBID = [result.id longLongValue];
             
                    m_nsstrUserName = [[NSString alloc] initWithString:result.first_name];
             
                    // Create a texture from the user's profile picture
                    m_pUserTexture = new System::TextureResource();
                    m_pUserTexture->CreateFromFBID(m_uPlayerFBID, 256, 256);
                }
             }];
        }
       
        void GameController::FB_ProcessIncomingURL(NSURL * targetURL)
        {
            // Process the incoming url and see if it's of value...
                      
            NSRange range = [targetURL.query rangeOfString:@"notif" options:NSCaseInsensitiveSearch];
            
            // If the url's query contains 'notif', we know it's coming from a notification - let's process it
            if(targetURL.query && range.location != NSNotFound)
            {
                // Yes the incoming URL was a notification
                FB_ProcessIncomingRequest(targetURL);
            }
          
            range = [targetURL.path rangeOfString:@"challenge_brag" options:NSCaseInsensitiveSearch];
            
            // If the url's path contains 'challenge_brag', we know it comes from a feed post
            if(targetURL.path && range.location != NSNotFound)
            {
                // Yes the incoming URL was a notification
                FB_ProcessIncomingFeed(targetURL);
            }
             
        }
        
        void GameController::FB_ProcessIncomingRequest(NSURL* targetURL)
        {
            // Extract the notification id
            NSArray *pairs = [targetURL.query componentsSeparatedByString:@"&"];
            NSMutableDictionary *queryParams = [[NSMutableDictionary alloc] init];
            for (NSString *pair in pairs) 
            {
                NSArray *kv = [pair componentsSeparatedByString:@"="];
                NSString *val = [[kv objectAtIndex:1]
                                 stringByReplacingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
                
                [queryParams setObject:val forKey:[kv objectAtIndex:0]];
                
            }
                
            NSString *requestIDsString = [queryParams objectForKey:@"request_ids"];
            NSArray *requestIDs = [requestIDsString componentsSeparatedByString:@","];
            
            FBRequest *req = [[FBRequest alloc] initWithSession:[FBSession activeSession] graphPath:[requestIDs objectAtIndex:0]];
            
            [req startWithCompletionHandler:^(FBRequestConnection *connection, id result, NSError *error) 
             {
             if (!error) 
             {
                
                if ([result objectForKey:@"from"]) 
                {
                    NSString *from = [[result objectForKey:@"from"] objectForKey:@"name"];
                    NSString *id = [[result objectForKey:@"from"] objectForKey:@"id"];
             
                    StartGame(true, true, from, id);
                }
             
             }
             
             }];
          
        }
        
        void GameController::FB_ProcessIncomingFeed(NSURL* targetURL)
        {
            // Here we process an incoming link that has launched the app via a feed post
            
            // Here is extract out the FBID component at the end of the brag, so 'challenge_brag_123456' becomes just 123456
            NSString* val = [[targetURL.path componentsSeparatedByString:@"challenge_brag_"] lastObject];
            
            FBRequest *req = [[FBRequest alloc] initWithSession:[FBSession activeSession] graphPath:val];
            
            // With the FBID extracted, we have enough information to go ahead and request the user's profile picture
            // But we also need their name, so here we make a request to http://graph.facebook.com/USER_ID to get their basic information
            [req startWithCompletionHandler:^(FBRequestConnection *connection, id result, NSError *error) 
            {
                // If the result came back okay with no errors...
                if (result && !error) 
                {
            
                    NSString *from = [result objectForKey:@"first_name"];
             
                    // We can start the game, 
                    StartGame(true, true, from, val);

                }
             
             }];
            
        }
        
        void GameController::FB_RequestWritePermissions()
        {
            // We need to request write permissions from Facebook
            static bool bHaveRequestedPublishPermissions = false;
            
            if (!bHaveRequestedPublishPermissions)
            {
                NSArray *permissions = [[NSArray alloc] initWithObjects:
                                        @"publish_actions", nil];
                
                [[FBSession activeSession] requestNewPublishPermissions:permissions defaultAudience:FBSessionDefaultAudienceFriends completionHandler:^(FBSession *session, NSError *error) {
                    NSLog(@"Reauthorized with publish permissions.");
                 }];
                
        
                
                bHaveRequestedPublishPermissions = true;
            }
            
            
        }
        
        void GameController::FB_SendScore(const int nScore)
        {
            // Make sure we have write permissions
            FB_RequestWritePermissions();
             
            NSMutableDictionary* params =   [NSMutableDictionary dictionaryWithObjectsAndKeys:
                                             [NSString stringWithFormat:@"%d", nScore], @"score",
                                             nil];
            
            NSLog(@"Fetching current score");
            
            // Get the score, and only send the updated score if it's highter
            [FBRequestConnection startWithGraphPath:[NSString stringWithFormat:@"%llu/scores", m_uPlayerFBID] parameters:params HTTPMethod:@"GET" completionHandler:^(FBRequestConnection *connection, id result, NSError *error) {
             
                if (result && !error) {
             
                    int nCurrentScore = [[[[result objectForKey:@"data"] objectAtIndex:0] objectForKey:@"score"] intValue];
             
                    NSLog(@"Current score is %d", nCurrentScore);
             
                    if (nScore > nCurrentScore) {
             
                        NSLog(@"Posting new score of %d", nScore);
             
                        [FBRequestConnection startWithGraphPath:[NSString stringWithFormat:@"%llu/scores", m_uPlayerFBID] parameters:params HTTPMethod:@"POST" completionHandler:^(FBRequestConnection *connection, id result, NSError *error) {
                         
                            NSLog(@"Score posted");
                        }];
                    }
                    else {
                        NSLog(@"Existing score is higher - not posting new score");
                    }
                }
             
             }];
            
            
            
            // Send our custom OG
            FB_SendOG();
        }
        
        
        void GameController::FB_GetScores()
        {
            [FBRequestConnection startWithGraphPath:[NSString stringWithFormat:@"%llu/scores?fields=score,user", kuFBAppID] parameters:nil HTTPMethod:@"GET" completionHandler:^(FBRequestConnection *connection, id result, NSError *error) {
            
                if (result && !error)
                {
                    int index = 0;
                    for (NSDictionary *dict in [result objectForKey:@"data"])
                    {
                        NSString *name = [[[[dict objectForKey:@"user"] objectForKey:@"name"] componentsSeparatedByString:@" "] objectAtIndex:0];
                        NSString *strScore = [dict objectForKey:@"score"];
             
                        m_pLeaderboardEntries[index].pFriendName.text = [NSString stringWithFormat:@"%d. %@", index+1, name];
                        m_pLeaderboardEntries[index].pFriendScore.text = [NSString stringWithFormat:@"Score: %@", strScore];

                        delete m_pLeaderboardEntries[index].pUserSprite;
                        m_pLeaderboardEntries[index].pUserSprite = NULL;
             
                        u64 uFriendID = [[[dict objectForKey:@"user"] objectForKey:@"id"] longLongValue];
                        m_pLeaderboardEntries[index].pUserTexture = new System::TextureResource();
                        m_pLeaderboardEntries[index].pUserTexture->CreateFromFBID(uFriendID, 64, 64);
    
                        index++;
                        if (index>5) {
                            break;
                        }
                    }
                }
             
            }];
        }
        
        
        void GameController::FB_SendAchievement(eGameAchievements achievement)
        {
            // Make sure we have write permissions
            FB_RequestWritePermissions();
            
            NSArray *achievementURLs = [NSArray arrayWithObjects:   @"http://www.friendsmash.com/opengraph/achievement_50.html",
                                                                    @"http://www.friendsmash.com/opengraph/achievement_100.html",
                                                                    @"http://www.friendsmash.com/opengraph/achievement_150.html",
                                                                    @"http://www.friendsmash.com/opengraph/achievement_200.html",
                                                                    @"http://www.friendsmash.com/opengraph/achievement_x3.html",
                                                                    nil];
            
            NSMutableDictionary* params =   [NSMutableDictionary dictionaryWithObjectsAndKeys:
                                             [NSString stringWithFormat:@"%@", [achievementURLs objectAtIndex:achievement]], @"achievement",
                                             nil];
            
            [FBRequestConnection startWithGraphPath:[NSString stringWithFormat:@"%llu/achievements", m_uPlayerFBID] parameters:params HTTPMethod:@"POST" completionHandler:^(FBRequestConnection *connection, id result, NSError *error) {
             }];
            
            
        }
        
        void GameController::FB_SendOG()
        {
            FBRequest* newAction = [[FBRequest alloc]initForPostWithSession:[FBSession activeSession] graphPath:[NSString stringWithFormat:@"me/friendsmashsample:smash?profile=%llu", m_uFriendFBID] graphObject:nil];
            
            FBRequestConnection* conn = [[FBRequestConnection alloc] init];
            
            [conn addRequest:newAction completionHandler:^(FBRequestConnection *connection, id result, NSError *error) {
             if(error) {
             NSLog(@"Sending OG Story Failed: %@", result[@"id"]);
             return;
             }
             
             NSLog(@"OG action ID: %@", result[@"id"]);
             }];
            [conn start];
        }
        
  
        void GameController::FB_SendRequest(const int nScore)
        {
            //FB_SendFilteredRequest(nScore);
            //return;
            
            // Enable this to turn on Frictionless Requests
            //[[appDelegate facebook] enableFrictionlessRequests];
            
  
            
   
            // Normally this won't be hardcoded but will be context specific, i.e. players you are in a match with, or players who recently played the game etc
            NSArray *suggestedFriends = [[NSArray alloc] initWithObjects:
                                         @"695755709", @"685145706", @"569496010", @"7963306",
            	                             nil];
    
            SBJsonWriter *jsonWriter = [SBJsonWriter new];
            NSDictionary *challenge =  [NSDictionary dictionaryWithObjectsAndKeys: [NSString stringWithFormat:@"%d", nScore], @"challenge_score", nil];
            NSString *challengeStr = [jsonWriter stringWithObject:challenge];
            
            
            // Create a dictionary of key/value pairs which are the parameters of the dialog
            
            // 1. No additional parameters provided - enables generic Multi-friend selector
            NSMutableDictionary* params =   [NSMutableDictionary dictionaryWithObjectsAndKeys:
            // 2. Optionally provide a 'to' param to direct the request at a specific user                               
                                            //@"286400088", @"to", // Ali
            // 3. Suggest friends the user may want to request, could be game context specific?                               
                                            [suggestedFriends componentsJoinedByString:@","], @"suggestions",
                                             challengeStr, @"data",
                                            nil];
                        
            FBFrictionlessRecipientCache *friendCache = [[FBFrictionlessRecipientCache alloc] init];
            [friendCache prefetchAndCacheForSession:nil];
            
            [FBWebDialogs presentRequestsDialogModallyWithSession:nil
                                                          message:[NSString stringWithFormat:@"I just smashed %d friends! Can you beat it?", nScore]
                                                            title:nil
                                                       parameters:params
                                                          handler:^(FBWebDialogResult result, NSURL *resultURL, NSError *error) {
             if (error) {
                    // Case A: Error launching the dialog or sending request.
                    NSLog(@"Error sending request.");
             } else {
                if (result == FBWebDialogResultDialogNotCompleted) {
                    // Case B: User clicked the "x" icon
                    NSLog(@"User canceled request.");
                } else {
                    NSLog(@"Request Sent.");
                }
             }}
             friendCache:friendCache];
    
            
             
        }
        
        void GameController::FB_SendFilteredRequest(const int nScore)
        {
            // Okay, we're going to filter our friends by their device, we're looking for friends with an iPhone or iPad
            
            // We're going to place these friends into this container
            NSMutableArray *deviceFilteredFriends = [[NSMutableArray alloc] init];
            
            // We request a list of our friends' names and devices
            [[FBRequest requestForGraphPath:@"me/friends?fields=name,devices"]
             startWithCompletionHandler:
             ^(FBRequestConnection *connection, 
               NSDictionary *result,
               NSError *error) 
             {
                // If we received a result with no errors...
                if (!error && result) 
                {
                    // Get the result
                    NSArray *resultData = [result objectForKey:@"data"];
                    
                    // Check we have some friends. If the player doesn't have any friends, they probably need to put down the demo app anyway, and go outside...
                    if ([resultData count] > 0) 
                    {
                        // Loop through the friends returned
                        for (NSDictionary *friendObject in resultData) 
                        {
                            // Check if devices info available
                            if ([friendObject objectForKey:@"devices"]) 
                            {
                                // Yep, we know what devices this friend has.. let's extract them
                                NSArray *deviceData = [friendObject objectForKey:@"devices"];
                                  
                                // Loop through the list of devices this friend has...
                                for (NSDictionary *deviceObject in deviceData) 
                                {
                                    // Check if there is a device match, in this case we're looking for iOS
                                    if ([@"iOS" isEqualToString: [deviceObject objectForKey:@"os"]]) 
                                    {
                                        // If there is a match, add it to the list - this friend has an iPhone or iPad. Hurrah!
                                        [deviceFilteredFriends addObject: [friendObject objectForKey:@"id"]];
                                        break;
                                    }
                                }
                            }
                        }
                    }
             
                    // Now we have a list of friends with an iOS device, we can send requests to them
            
                    // We create our parameter dictionary as we did before
                    NSMutableDictionary* params = [NSMutableDictionary dictionaryWithObjectsAndKeys:nil];
            
                    // We have the same list of suggested friends
                    NSArray *suggestedFriends = [[NSArray alloc] initWithObjects:
                                                 @"695755709", @"685145706", @"569496010", @"7963306", 
                                                 nil];
             
             
                    // Of course, not all of our suggested friends will have iPhones or iPads - we need to filter them down
                    NSMutableArray *validSuggestedFriends = [[NSMutableArray alloc] init];
             
                    // So, we loop through each suggested friend
                    for (NSString* suggestedFriend in suggestedFriends)
                    {
                        // If they are on our device filtered list, we know they have an iOS device
                        if ([deviceFilteredFriends containsObject:suggestedFriend])
                        {
                            // So we can call them valid
                            [validSuggestedFriends addObject:suggestedFriend];
                        }
                    }
                
                    // If at least one of our suggested friends had an iOS device...
                    if ([deviceFilteredFriends count] > 0) 
                    {
                        // We add them to the suggest friend param of the dialog
                        NSString *selectIDsStr = [validSuggestedFriends componentsJoinedByString:@","];
                        [params setObject:selectIDsStr forKey:@"suggestions"];
                    }
             
                     FBFrictionlessRecipientCache *friendCache = [[FBFrictionlessRecipientCache alloc] init];
                     [friendCache prefetchAndCacheForSession:nil];
                     
                     [FBWebDialogs presentRequestsDialogModallyWithSession:nil
                                                                   message:[NSString stringWithFormat:@"I just smashed %d friends! Can you beat it?", nScore]
                                                                     title:nil
                                                                parameters:params
                                                                   handler:^(FBWebDialogResult result, NSURL *resultURL, NSError *error) {
                      if (error) {
                        NSLog(@"Error sending request.");
                      } else {
                        if (result == FBWebDialogResultDialogNotCompleted) {
                            NSLog(@"User canceled request.");
                        } else {
                            NSLog(@"Request Sent.");
                        }
                      }}
                                                                friendCache:friendCache];
            
                 }
             }]; 
        }
        
        void GameController::FB_SendBrag(const int nScore)
        {
          

          // This function will invoke the Feed Dialog to post to a user's Timeline and News Feed
          // It will attemnt to use the Facebook Native Share dialog
          // If that's not supported we'll fall back to the web based dialog.
          
          NSString *linkURL = [NSString stringWithFormat:@"https://www.friendsmash.com/challenge_brag_%llu", m_uPlayerFBID];
          NSString *pictureURL = @"http://www.friendsmash.com/images/logo_large.jpg";
          
          // Prepare the native share dialog parameters
          FBShareDialogParams *shareParams = [[FBShareDialogParams alloc] init];
          shareParams.link = [NSURL URLWithString:linkURL];
          shareParams.name = @"Checkout my Friend Smash greatness!";
          shareParams.caption= @"Come smash me back!";
          shareParams.picture= [NSURL URLWithString:pictureURL];
          shareParams.description =
            [NSString stringWithFormat:@"I just smashed %d friends! Can you beat my score?", nScore];
          
          if ([FBDialogs canPresentShareDialogWithParams:shareParams]){
            
            [FBDialogs presentShareDialogWithParams:shareParams
                                        clientState:nil
                                            handler:^(FBAppCall *call, NSDictionary *results, NSError *error) {
               if(error) {
                  NSLog(@"Error publishing story.");
               } else if (results[@"completionGesture"] && [results[@"completionGesture"] isEqualToString:@"cancel"]) {
                  NSLog(@"User canceled story publishing.");
               } else {
                  NSLog(@"Story published.");
               }
              }];
            
          }else {
         
              // Prepare the web dialog parameters
              NSDictionary *params = @{
                @"name" : shareParams.name,
                @"caption" : shareParams.caption,
                @"description" : shareParams.description,
                @"picture" : pictureURL,
                @"link" : linkURL
              };
               
              // Invoke the dialog
              [FBWebDialogs presentFeedDialogModallyWithSession:nil
                                                     parameters:params
                                                        handler:
               ^(FBWebDialogResult result, NSURL *resultURL, NSError *error) {
               if (error) {
                  NSLog(@"Error publishing story.");
               } else {
                  if (result == FBWebDialogResultDialogNotCompleted) {
                      NSLog(@"User canceled story publishing.");
                  } else {
                      NSLog(@"Story published.");
                  }
               }}];
            }
        }
        
        
        void GameController::FB_Logout()
        {
            // Log out of Facebook and reset our session
            [[FBSession activeSession] closeAndClearTokenInformation];
            [FBSession setActiveSession:nil];
            
            m_pLoginButtonSprite->SetDraw(true);
            
            m_pPlayButtonSprite->SetDraw(false);
            m_pLogoutButtonSprite->SetDraw(false);
            m_pChallengeButtonSprite->SetDraw(false);
            m_pBragButtonSprite->SetDraw(false);
            m_pUserImageSprite->SetDraw(false);
            m_pScoresButtonSprite->SetDraw(false);
            
            m_labelName.text = [NSString stringWithFormat:@"Welcome, Player"];
            
            m_labelNameStatus.hidden = YES;
            m_labelFriendName.hidden = YES;
            m_labelScore.hidden = YES;
            
            m_kGameState = kGAMESTATE_FRONTSCREEN_LOGGEDOUT;
        }
        
    
    }
		
}