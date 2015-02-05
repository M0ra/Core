SET @SQUIRE=33238;
SET @GRUNT=33239;
SET @GOSSIPSQUIRE=10318; -- From sniff
SET @GOSSIPGRUNT=10317; -- Guess 10319 is taken textid 14330 "Missng US text"
-- Add Spell Script name
DELETE FROM `spell_script_names` WHERE `spell_id` = 67039;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUE 
(67039, 'spell_gen_mount_check');
-- spell link to apply tired debuff
DELETE FROM `spell_linked_spell` WHERE `spell_effect` = 67401;
INSERT INTO `spell_linked_spell` (`spell_trigger`,`spell_effect`, `type`, `comment`) VALUES
(-67368, 67401, 0, 'Argent Squire - Bank - Tired'),
(-67377, 67401, 0, 'Argent Squire - Shop - Tired'),
(-67376, 67401, 0, 'Argent Squire - Mail - Tired');
-- Add gossip menu id for argent squire and gruntling and change npc flag
UPDATE `creature_template` SET `gossip_menu_id`=@GOSSIPSQUIRE, `npcflag`=129,`ScriptName`='npc_argent_squire' WHERE `entry` = @SQUIRE;
UPDATE `creature_template` SET `gossip_menu_id`=@GOSSIPGRUNT, `npcflag`=129,`ScriptName`='npc_argent_squire' WHERE `entry` = @GRUNT;
-- Add gosip for argent squire and gruntling text already in db
DELETE FROM `gossip_menu` WHERE `entry`IN (@GOSSIPSQUIRE,@GOSSIPGRUNT);
INSERT INTO `gossip_menu` (`entry`, `text_id`) VALUES 
(@GOSSIPSQUIRE, 14324),
(@GOSSIPGRUNT, 14372);
-- Add gosip menu options for argent squire and gruntling
DELETE FROM `gossip_menu_option` WHERE `menu_id`IN (@GOSSIPSQUIRE,@GOSSIPGRUNT);
INSERT INTO `gossip_menu_option` (`menu_id`, `id`, `option_icon`, `option_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `box_coded`, `box_money`, `box_text`) VALUES
(@GOSSIPSQUIRE, 0, 6, 'Visit a bank', 9, 1, 0, 0, 0, 0, ''),
(@GOSSIPSQUIRE, 1, 1, 'Visit a trader', 3, 1, 0, 0, 0, 0, ''),
(@GOSSIPSQUIRE, 2, 0, 'Visit a mailbox', 1, 1, 0, 0, 0, 0, ''), 
(@GOSSIPSQUIRE, 3, 0, 'Darnassus Champion\'s Pennant', 1, 1, 0, 0, 0, 0, ''),
(@GOSSIPSQUIRE, 4, 0, 'Exodar Champion\'s Pennant', 1, 1, 0, 0, 0, 0, ''),
(@GOSSIPSQUIRE, 5, 0, 'Gnomeeregan Champion\'s Pennant', 1, 1, 0, 0, 0, 0, ''),
(@GOSSIPSQUIRE, 6, 0, 'Ironforge Champion\'s Pennant', 1, 1, 0, 0, 0, 0, ''),
(@GOSSIPSQUIRE, 7, 0, 'Stormwind Champion\'s Pennant', 1, 1, 0, 0, 0, 0, ''),
(@GOSSIPGRUNT, 0, 6, 'Visit a bank', 9, 1, 0, 0, 0, 0, ''),
(@GOSSIPGRUNT, 1, 1, 'Visit a trader', 3, 1, 0, 0, 0, 0, ''),
(@GOSSIPGRUNT, 2, 0, 'Visit a mailbox', 1, 1, 0, 0, 0, 0, ''), 
(@GOSSIPGRUNT, 3, 0, 'Darkspear Champion\'s Pennant', 1, 1, 0, 0, 0, 0, ''),
(@GOSSIPGRUNT, 4, 0, 'Forsaken Champion\'s Pennant', 1, 1, 0, 0, 0, 0, ''),
(@GOSSIPGRUNT, 5, 0, 'Orgrimmar Champion\'s Pennant', 1, 1, 0, 0, 0, 0, ''),
(@GOSSIPGRUNT, 6, 0, 'Silvermoon Champion\'s Pennant', 1, 1, 0, 0, 0, 0, ''),
(@GOSSIPGRUNT, 7, 0, 'Thunder Bluff Champion\'s Pennant', 1, 1, 0, 0, 0, 0, '');
-- Add vendor items for argent squire and gruntling
DELETE FROM `npc_vendor` WHERE `entry` IN (@SQUIRE,@GRUNT);
INSERT INTO `npc_vendor` (`entry`, `slot`, `item`, `maxcount`, `incrtime`, `ExtendedCost`) VALUES 
(@SQUIRE, 0, 33449, 0, 0, 0),-- Crusty Flatbread
(@SQUIRE, 0, 33443, 0, 0, 0),-- Sour Goat Cheese
(@SQUIRE, 0, 33451, 0, 0, 0),-- Fillet of Icefin
(@SQUIRE, 0, 35949, 0, 0, 0),-- Tundra Berries
(@SQUIRE, 0, 33452, 0, 0, 0),-- Honey-Spiced Lichen
(@SQUIRE, 0, 33454, 0, 0, 0),-- Salted Venison
(@SQUIRE, 0, 35950, 0, 0, 0),-- Sweet Potato Bread
(@SQUIRE, 0, 35952, 0, 0, 0),-- Briny Hardcheese
(@SQUIRE, 0, 35951, 0, 0, 0),-- Poached Emperor Salmon
(@SQUIRE, 0, 35948, 0, 0, 0),-- Savory Snowplum
(@SQUIRE, 0, 35947, 0, 0, 0),-- Sparkling Frostcap
(@SQUIRE, 0, 35953, 0, 0, 0),-- Mead Basted Caribou
(@SQUIRE, 0, 17034, 0, 0, 0),-- Maple Seed
(@SQUIRE, 0, 17035, 0, 0, 0),-- Stranglethorn Seed
(@SQUIRE, 0, 17036, 0, 0, 0),-- Ashwood Seed
(@SQUIRE, 0, 17037, 0, 0, 0),-- Hornbeam Seed
(@SQUIRE, 0, 17038, 0, 0, 0),-- Ironwood Seed
(@SQUIRE, 0, 22147, 0, 0, 0),-- Flintwood Seed
(@SQUIRE, 0, 17031, 0, 0, 0),-- Rune of Teleportation
(@SQUIRE, 0, 17032, 0, 0, 0),-- Rune of Portals
(@SQUIRE, 0, 17020, 0, 0, 0),-- Arcane Powder
(@SQUIRE, 0, 17030, 0, 0, 0),-- Ankh
(@SQUIRE, 0, 17033, 0, 0, 0),-- Symbol of Divinity
(@SQUIRE, 0, 17028, 0, 0, 0),-- Holy Candle
(@SQUIRE, 0, 17029, 0, 0, 0),-- Sacred Candle
(@SQUIRE, 0, 17021, 0, 0, 0),-- Wild Berries
(@SQUIRE, 0, 17026, 0, 0, 0),-- Wild Thornroot
(@SQUIRE, 0, 22148, 0, 0, 0),-- Wild Quillvine
(@SQUIRE, 0, 5565, 0, 0, 0),-- Infernal Stone
(@SQUIRE, 0, 16583, 0, 0, 0),-- Demonic Figurine
(@SQUIRE, 0, 21177, 0, 0, 0),-- Symbol of Kings
(@SQUIRE, 0, 37201, 0, 0, 0),-- Corpse Dust
(@SQUIRE, 0, 44614, 0, 0, 0),-- Starleaf Seed
(@SQUIRE, 0, 44615, 0, 0, 0),-- Devout Candle
(@SQUIRE, 0, 44605, 0, 0, 0),-- Wild Spineleaf
(@SQUIRE, 0, 28056, 0, 0, 0),-- Blackflight Arrow
(@SQUIRE, 0, 41586, 0, 0, 0),-- Terrorshaft Arrow
(@SQUIRE, 0, 28061, 0, 0, 0),-- Ironbite Shells
(@SQUIRE, 0, 41584, 0, 0, 0),-- Frostbite Shells
(@SQUIRE, 0, 29014, 0, 0, 0),-- Blacksteel Throwing Dagger
(@SQUIRE, 0, 29013, 0, 0, 0),-- Jagged Throwing Axe
(@SQUIRE, 0, 8928, 0, 0, 0),-- Instant Poison VI
(@SQUIRE, 0, 43230, 0, 0, 0),-- Instant Poison VIII
(@SQUIRE, 0, 22053, 0, 0, 0),-- Deadly Poison VI
(@SQUIRE, 0, 43232, 0, 0, 0),-- Deadly Poison VIII
(@SQUIRE, 0, 43234, 0, 0, 0),-- Wound Poison VI
(@SQUIRE, 0, 21835, 0, 0, 0),-- Anesthetic Poison
(@GRUNT, 0, 33449, 0, 0, 0),-- Crusty Flatbread
(@GRUNT, 0, 33443, 0, 0, 0),-- Sour Goat Cheese
(@GRUNT, 0, 33451, 0, 0, 0),-- Fillet of Icefin
(@GRUNT, 0, 35949, 0, 0, 0),-- Tundra Berries
(@GRUNT, 0, 33452, 0, 0, 0),-- Honey-Spiced Lichen
(@GRUNT, 0, 33454, 0, 0, 0),-- Salted Venison
(@GRUNT, 0, 35950, 0, 0, 0),-- Sweet Potato Bread
(@GRUNT, 0, 35952, 0, 0, 0),-- Briny Hardcheese
(@GRUNT, 0, 35951, 0, 0, 0),-- Poached Emperor Salmon
(@GRUNT, 0, 35948, 0, 0, 0),-- Savory Snowplum
(@GRUNT, 0, 35947, 0, 0, 0),-- Sparkling Frostcap
(@GRUNT, 0, 35953, 0, 0, 0),-- Mead Basted Caribou
(@GRUNT, 0, 17034, 0, 0, 0),-- Maple Seed
(@GRUNT, 0, 17035, 0, 0, 0),-- Stranglethorn Seed
(@GRUNT, 0, 17036, 0, 0, 0),-- Ashwood Seed
(@GRUNT, 0, 17037, 0, 0, 0),-- Hornbeam Seed
(@GRUNT, 0, 17038, 0, 0, 0),-- Ironwood Seed
(@GRUNT, 0, 22147, 0, 0, 0),-- Flintwood Seed
(@GRUNT, 0, 17031, 0, 0, 0),-- Rune of Teleportation
(@GRUNT, 0, 17032, 0, 0, 0),-- Rune of Portals
(@GRUNT, 0, 17020, 0, 0, 0),-- Arcane Powder
(@GRUNT, 0, 17030, 0, 0, 0),-- Ankh
(@GRUNT, 0, 17033, 0, 0, 0),-- Symbol of Divinity
(@GRUNT, 0, 17028, 0, 0, 0),-- Holy Candle
(@GRUNT, 0, 17029, 0, 0, 0),-- Sacred Candle
(@GRUNT, 0, 17021, 0, 0, 0),-- Wild Berries
(@GRUNT, 0, 17026, 0, 0, 0),-- Wild Thornroot
(@GRUNT, 0, 22148, 0, 0, 0),-- Wild Quillvine
(@GRUNT, 0, 5565, 0, 0, 0),-- Infernal Stone
(@GRUNT, 0, 16583, 0, 0, 0),-- Demonic Figurine
(@GRUNT, 0, 21177, 0, 0, 0),-- Symbol of Kings
(@GRUNT, 0, 37201, 0, 0, 0),-- Corpse Dust
(@GRUNT, 0, 44614, 0, 0, 0),-- Starleaf Seed
(@GRUNT, 0, 44615, 0, 0, 0),-- Devout Candle
(@GRUNT, 0, 44605, 0, 0, 0),-- Wild Spineleaf
(@GRUNT, 0, 28056, 0, 0, 0),-- Blackflight Arrow
(@GRUNT, 0, 41586, 0, 0, 0),-- Terrorshaft Arrow
(@GRUNT, 0, 28061, 0, 0, 0),-- Ironbite Shells
(@GRUNT, 0, 41584, 0, 0, 0),-- Frostbite Shells
(@GRUNT, 0, 29014, 0, 0, 0),-- Blacksteel Throwing Dagger
(@GRUNT, 0, 29013, 0, 0, 0),-- Jagged Throwing Axe
(@GRUNT, 0, 8928, 0, 0, 0),-- Instant Poison VI
(@GRUNT, 0, 43230, 0, 0, 0),-- Instant Poison VIII
(@GRUNT, 0, 22053, 0, 0, 0),-- Deadly Poison VI
(@GRUNT, 0, 43232, 0, 0, 0),-- Deadly Poison VIII
(@GRUNT, 0, 43234, 0, 0, 0),-- Wound Poison VI
(@GRUNT, 0, 21835, 0, 0, 0);-- Anesthetic Poison
-- Add conditions for gossip options
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=15 AND `SourceGroup` IN (@GOSSIPSQUIRE,@GOSSIPGRUNT);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES 
(15, @GOSSIPSQUIRE, 0, 0, 1, 17, 0, 3736, 0, 0, 0, 0, '', 'Argent squire show Visit a bank if player has achievement pony up'),
(15, @GOSSIPSQUIRE, 0, 0, 1, 1, 1, 67401, 0, 0, 1, 0, '', 'Argent squire show Visit a bank no tired aura'),
(15, @GOSSIPSQUIRE, 0, 0, 1, 1, 1, 67377, 0, 0, 1, 0, '', 'Argent squire show Visit a bank no shop aura'),
(15, @GOSSIPSQUIRE, 0, 0, 1, 1, 1, 67376, 0, 0, 1, 0, '', 'Argent squire show Visit a bank no postman aura'),
(15, @GOSSIPSQUIRE, 1, 0, 2, 17, 0, 3736, 0, 0, 0, 0, '', 'Argent squire show Visit a trader if player has achievement pony up'),
(15, @GOSSIPSQUIRE, 1, 0, 2, 1, 1, 67401, 0, 0, 1, 0, '', 'Argent squire show Visit a trader no tired aura'),
(15, @GOSSIPSQUIRE, 1, 0, 2, 1, 1, 67368, 0, 0, 1, 0, '', 'Argent squire show Visit a trader no bank errand aura'),
(15, @GOSSIPSQUIRE, 1, 0, 2, 1, 1, 67376, 0, 0, 1, 0, '', 'Argent squire show Visit a trader no postman aura'),
(15, @GOSSIPSQUIRE, 2, 0, 3, 17, 0, 3736, 0, 0, 0, 0, '', 'Argent squire show Visit a mailbox if player has achievement pony up'),
(15, @GOSSIPSQUIRE, 2, 0, 3, 1, 1, 67401, 0, 0, 1, 0, '', 'Argent squire show Visit a mailbox no tired aura'),
(15, @GOSSIPSQUIRE, 2, 0, 3, 1, 1, 67368, 0, 0, 1, 0, '', 'Argent squire show Visit a mailbox no bank errand aura'),
(15, @GOSSIPSQUIRE, 2, 0, 3, 1, 1, 67377, 0, 0, 1, 0, '', 'Argent squire show Visit a mailbox no shop aura'),
(15, @GOSSIPSQUIRE, 3, 0, 0, 8, 0, 13725, 0, 0, 0, 0, '', 'Argent squire show Darnassus Champion\'s Pennant if player has completed Champion of Darnassus'),
(15, @GOSSIPSQUIRE, 4, 0, 0, 8, 0, 13724, 0, 0, 0, 0, '', 'Argent squire show Exodar Champion\'s Pennant if player has completed Champion of the Exodar'),
(15, @GOSSIPSQUIRE, 5, 0, 0, 8, 0, 13723, 0, 0, 0, 0, '', 'Argent squire show Gnomeeregan Champion\'s Pennant if player has completed Champion of Gnomeregan'),
(15, @GOSSIPSQUIRE, 6, 0, 0, 8, 0, 13713, 0, 0, 0, 0, '', 'Argent squire show Ironforge Champion\'s Pennant if player has completed Champion of Ironforge'),
(15, @GOSSIPSQUIRE, 7, 0, 0, 8, 0, 13699, 0, 0, 0, 0, '', 'Argent squire show Stormwind Champion\'s Pennant if player has completed Champion of Stormwind'),
(15, @GOSSIPGRUNT, 0, 0, 1, 17, 0, 3736, 0, 0, 0, 0, '', 'Argent Gruntling show Visit a bank if player has achievement pony up'),
(15, @GOSSIPGRUNT, 0, 0, 1, 1, 1, 67401, 0, 0, 1, 0, '', 'Argent Gruntling show Visit a bank no tired aura'),
(15, @GOSSIPGRUNT, 0, 0, 1, 1, 1, 67377, 0, 0, 1, 0, '', 'Argent Gruntling show Visit a bank no shop aura'),
(15, @GOSSIPGRUNT, 0, 0, 1, 1, 1, 67376, 0, 0, 1, 0, '', 'Argent Gruntling show Visit a bank no Postman aura'),
(15, @GOSSIPGRUNT, 1, 0, 2, 17, 0, 3736, 0, 0, 0, 0, '', 'Argent Gruntling show Visit a trader if player has achievement pony up'),
(15, @GOSSIPGRUNT, 1, 0, 2, 1, 1, 67401, 0, 0, 1, 0, '', 'Argent Gruntling show Visit a trader no tired aura'),
(15, @GOSSIPGRUNT, 1, 0, 2, 1, 1, 67368, 0, 0, 1, 0, '', 'Argent Gruntling show Visit a trader no bank errand aura'),
(15, @GOSSIPGRUNT, 1, 0, 2, 1, 1, 67376, 0, 0, 1, 0, '', 'Argent Gruntling show Visit a trader no Postman aura'),
(15, @GOSSIPGRUNT, 2, 0, 3, 17, 0, 3736, 0, 0, 0, 0, '', 'Argent Gruntling show Visit a mailbox if player has achievement pony up'),
(15, @GOSSIPGRUNT, 2, 0, 3, 1, 1, 67401, 0, 0, 1, 0, '', 'Argent Gruntling show Visit a trader no tired aura'),
(15, @GOSSIPGRUNT, 2, 0, 3, 1, 1, 67368, 0, 0, 1, 0, '', 'Argent Gruntling show Visit a trader no bank errand aura'),
(15, @GOSSIPGRUNT, 2, 0, 3, 1, 1, 67377, 0, 0, 1, 0, '', 'Argent Gruntling show Visit a trader no shop aura'),
(15, @GOSSIPGRUNT, 3, 0, 0, 8, 0, 13727, 0, 0, 0, 0, '', 'Argent Gruntling show Darkspear Champion\'s Pennant if player has completed Champion of Sen\'jin'),
(15, @GOSSIPGRUNT, 4, 0, 0, 8, 0, 13729, 0, 0, 0, 0, '', 'Argent Gruntling show Forsaken Champion\'s Pennant if player has completed Champion of the Undercity'),
(15, @GOSSIPGRUNT, 5, 0, 0, 8, 0, 13726, 0, 0, 0, 0, '', 'Argent Gruntling show Orgrimmar Champion\'s Pennant if player has completed Champion of Orgrimmar'),
(15, @GOSSIPGRUNT, 6, 0, 0, 8, 0, 13731, 0, 0, 0, 0, '', 'Argent Gruntling show Silvermoon Champion\'s Pennant if player has completed Champion of Silvermoon City'),
(15, @GOSSIPGRUNT, 7, 0, 0, 8, 0, 13728, 0, 0, 0, 0, '', 'Argent Gruntling show Thunder Bluff Champion\'s Pennant if player has completed Champion of Thunder Bluff');
