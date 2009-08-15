CREATE TABLE IF NOT EXISTS `booklib` (
  `Title` varchar(99) character set cp1251 NOT NULL default '',
  `Filename` varchar(12) character set latin1 collate latin1_general_ci NOT NULL default '',
  `Date` date NOT NULL default '0000-00-00',
  `Path` varchar(99) character set latin1 collate latin1_general_ci NOT NULL default '',
  `Size` int(11) NOT NULL default '0',
  `Disc` int(11) NOT NULL default '0',
  `Seq` varchar(99) character set cp1251 NOT NULL default '',
  `G1` varchar(99) character set cp1251 NOT NULL default '',
  `G2` varchar(99) character set cp1251 NOT NULL default '',
  `G3` varchar(99) character set cp1251 NOT NULL default '',
  `A1N1` varchar(99) character set cp1251 NOT NULL default '',
  `A1N2` varchar(99) character set cp1251 NOT NULL default '',
  `SeqId` int(11) NOT NULL default '0',
  `SeqN` int(11) NOT NULL default '0',
  `A2N1` varchar(99) character set cp1251 NOT NULL default '',
  `A2N2` varchar(99) character set cp1251 NOT NULL default '',
  `Notes` varchar(254) character set cp1251 NOT NULL default '',
  `ID` int(11) NOT NULL auto_increment,
  PRIMARY KEY  (`ID`),
  KEY `Title` (`Title`),
  KEY `Filename` (`Filename`),
  KEY `Date` (`Date`),
  KEY `Path` (`Path`),
  KEY `Size` (`Size`),
  KEY `Seq` (`Seq`),
  KEY `G1` (`G1`),
  KEY `G2` (`G2`),
  KEY `G3` (`G3`),
  KEY `A1N1` (`A1N1`),
  KEY `A1N2` (`A1N2`),
  KEY `A2N1` (`A2N1`),
  KEY `A2N2` (`A2N2`)
) DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE IF NOT EXISTS `libactions` (
  `ActionID` int(11) NOT NULL auto_increment,
  `UserName` varchar(60) character set utf8 collate utf8_bin NOT NULL default '',
  `Time` timestamp NOT NULL default CURRENT_TIMESTAMP,
  `ActionSQL` text  NOT NULL,
  `ActionDesc` text  NOT NULL,
  `ActionUndo` text  NOT NULL,
  `BookId` int(11) NOT NULL,
  PRIMARY KEY  (`ActionID`),
  KEY `UserName` (`UserName`,`Time`),
  KEY `Time` (`Time`),
  KEY `BookId` (`BookId`)
) DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE IF NOT EXISTS `libavtor` (
  `BookId` int(10) unsigned NOT NULL default '0',
  `AvtorId` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`BookId`,`AvtorId`),
  KEY `iav` (`AvtorId`)
) DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE IF NOT EXISTS `libtranslator` (
  `BookId` int(10) unsigned NOT NULL default '0',
  `TranslatorId` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY `u` (`BookId`,`TranslatorId`),
  KEY `iav` (`TranslatorId`)
) DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE IF NOT EXISTS `libavtoraliase` (
  `AliaseId` int(11) NOT NULL auto_increment,
  `BadId` int(11) NOT NULL default '0',
  `GoodId` int(11) NOT NULL default '0',
  PRIMARY KEY  (`AliaseId`),
  KEY `u` (`BadId`,`GoodId`),
  KEY `BadId` (`BadId`),
  KEY `GoodId` (`GoodId`)
) DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci COMMENT='таблица замены авторов';

CREATE TABLE IF NOT EXISTS `libavtorname` (
  `AvtorId` int(10) unsigned NOT NULL auto_increment,
  `FirstName` varchar(99) character set utf8 NOT NULL default '',
  `MiddleName` varchar(99) character set utf8 NOT NULL default '',
  `LastName` varchar(99) character set utf8 NOT NULL default '',
  `NickName` varchar(33) character set utf8 NOT NULL default '',
  `NoDonate` char(1)  NOT NULL default '' COMMENT 'п...о - бе...м, 1 - нет',
  `uid` int(11) NOT NULL default '0',
  `WebPay` varchar(255)  NOT NULL default '',
  `Email` varchar(255)  NOT NULL default '',
  `Homepage` varchar(255)  NOT NULL default '',
   `Blocked` char(1) NOT NULL,
  PRIMARY KEY  USING BTREE (`AvtorId`),
  KEY `FirstName` (`FirstName`(20)),
  KEY `LastName` (`LastName`(20)),
  KEY `NoDonate` (`NoDonate`),
  KEY `Email` (`Email`),
  KEY `Homepage` (`Homepage`),
  FULLTEXT KEY `FirstName_2` (`FirstName`,`MiddleName`,`LastName`)
) DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

 CREATE TABLE IF NOT EXISTS `libbook` (
   `BookId` int(10) unsigned NOT NULL auto_increment,
   `FileName` varchar(12) character set latin1 collate latin1_general_ci NOT NULL default '',
   `FileSize` int(10) unsigned NOT NULL default '0',
   `Time` timestamp NOT NULL default CURRENT_TIMESTAMP,
   `Title` varchar(254)  NOT NULL default '',
   `Title1` varchar(254)  NOT NULL default '',
   `Lang` char(2)  NOT NULL default 'ru',
   `FileType` char(4)  NOT NULL default 'fb2',
   `Year` smallint(6) NOT NULL default '0',
   `Deleted` char(1)  NOT NULL default '',
   `Ver` varchar(8) character set utf8 NOT NULL default '',
   `FileAuthor` varchar(64) character set utf8 NOT NULL,
   `N` int(10) unsigned NOT NULL default '0',
   `keywords` varchar(255) character set utf8 NOT NULL,
   `Blocked` char(1) NOT NULL,
   PRIMARY KEY  (`BookId`),
   UNIQUE KEY `FileName` (`FileName`),
   KEY `Title` (`Title`),
   KEY `Title1` (`Title1`),
   KEY `Year` (`Year`),
   KEY `Deleted` (`Deleted`),
   KEY `SeqId` (`SeqId`),
   KEY `FileType` (`FileType`),
   KEY `Lang` (`Lang`),
   KEY `FileSize` (`FileSize`),
   KEY `FileAuthor` (`FileAuthor`),
   KEY `N` (`N`),
) DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE IF NOT EXISTS `libcache` (
  `BookId` int(11) NOT NULL default '0',
  `TOC` text  NOT NULL,
  `Cover` varchar(255)  NOT NULL default '',
  `Time` timestamp NOT NULL default CURRENT_TIMESTAMP,
  PRIMARY KEY  (`BookId`)
) DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE IF NOT EXISTS `libcache1` (
  `BookId` int(11) NOT NULL default '0',
  `Advise` varchar(255) NOT NULL default '',
  `Time` timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  PRIMARY KEY  (`BookId`)
) DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `libdonations` (
  `DonateId` int(11) NOT NULL auto_increment,
  `uid` int(11) NOT NULL default '0',
  `Sum` int(11) NOT NULL default '0',
  `Type` char(1) NOT NULL default '',
  `AvtorId` int(11) NOT NULL default '0',
  `PayType` char(10) NOT NULL default '' COMMENT 'а?аЋајб?б?а∆аЌајб? б?а»б?б?а≈аћај',
  `Time` timestamp NOT NULL default CURRENT_TIMESTAMP,
  PRIMARY KEY  (`DonateId`)
) DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE IF NOT EXISTS `libgenre` (
  `Id` int(10) unsigned NOT NULL auto_increment,
  `BookId` int(10) unsigned NOT NULL default '0',
  `GenreId` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`Id`),
  UNIQUE KEY `u` (`BookId`,`GenreId`),
  KEY `igenre` (`GenreId`),
  KEY `ibook` (`BookId`)
) DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci ROW_FORMAT=DYNAMIC;

CREATE TABLE IF NOT EXISTS `libgenrelist` (
  `GenreId` int(10) unsigned NOT NULL auto_increment,
  `GenreCode` varchar(45)  NOT NULL default '',
  `GenreDesc` varchar(99)  NOT NULL default '',
  `GenreMeta` varchar(45)  NOT NULL default '',
  PRIMARY KEY  USING BTREE (`GenreId`,`GenreCode`),
  KEY `meta` (`GenreMeta`)
) DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE IF NOT EXISTS `libjoinedbooks` (
  `Id` int(11) NOT NULL auto_increment,
  `Time` timestamp NOT NULL default CURRENT_TIMESTAMP,
  `BadId` int(11) NOT NULL default '0',
  `GoodId` int(11) NOT NULL default '0',
  PRIMARY KEY  (`Id`),
  UNIQUE KEY `BadId` (`BadId`),
  KEY `Time` (`Time`),
  KEY `GoodId` (`GoodId`)
) DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE IF NOT EXISTS `liblog` (
  `ID` int(10) unsigned NOT NULL auto_increment,
  `UserId` char(15) character set ascii collate ascii_bin NOT NULL default '',
  `Time` timestamp NOT NULL default CURRENT_TIMESTAMP,
  `BookId` int(11) NOT NULL default '0',
  PRIMARY KEY  (`ID`),
  KEY `User` (`UserId`),
  KEY `Time` (`Time`),
  KEY `url` (`BookId`)
) DEFAULT CHARSET=utf8 DELAY_KEY_WRITE=1;

CREATE TABLE IF NOT EXISTS `libmass` (
  `ID` int(10) unsigned NOT NULL auto_increment,
  `Time` timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  `Books` varchar(1000) NOT NULL,
  `N` int(11) NOT NULL,
  PRIMARY KEY  (`ID`),
  UNIQUE KEY `Books` (`Books`),
  KEY `Time` (`Time`),
  KEY `N` (`N`)
) DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE IF NOT EXISTS `libpolka` (
  `Id` int(10) unsigned NOT NULL auto_increment,
  `Time` timestamp NOT NULL default CURRENT_TIMESTAMP,
  `BookId` int(11) NOT NULL default '0',
  `AvtorId` int(11) NOT NULL default '0',
  `UserId` int(11) NOT NULL default '0',
  `Text` text character set utf8 NOT NULL,
  `Flag` char(1) NOT NULL default '',
  PRIMARY KEY  (`Id`),
  UNIQUE KEY `ub` (`BookId`,`UserId`),
  KEY `BookId` (`BookId`),
  KEY `AvtorId` (`AvtorId`),
  KEY `User` (`UserId`)
) DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE IF NOT EXISTS `libquality` (
  `BookId` int(11) NOT NULL,
  `uid` int(11) NOT NULL,
  `q` char(1) NOT NULL,
  UNIQUE KEY `ub` (`BookId`,`uid`),
  KEY `BookId` (`BookId`),
  KEY `uid` (`uid`),
  KEY `q` (`q`)
) DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE IF NOT EXISTS `librate` (
  `ID` int(11) NOT NULL auto_increment,
  `BookId` int(11) NOT NULL,
  `UserId` int(11) NOT NULL,
  `Rate` char(1) NOT NULL,
  PRIMARY KEY  (`ID`),
  UNIQUE KEY `BookId` (`BookId`,`UserId`),
  KEY `UserId` (`UserId`)
) DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE IF NOT EXISTS `libreaded` (
  `Time` timestamp NOT NULL default CURRENT_TIMESTAMP,
  `BookId` int(11) NOT NULL,
  `UserId` int(11) NOT NULL,
  PRIMARY KEY  (`BookId`,`UserId`),
  KEY `Time` (`Time`),
  KEY `UserId` (`UserId`)
) DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE IF NOT EXISTS `libseqname` (
  `SeqId` int(10) unsigned NOT NULL auto_increment,
  `SeqName` varchar(254) NOT NULL default '',
  PRIMARY KEY  (`SeqId`),
  UNIQUE KEY `SeqName` (`SeqName`)
) DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE IF NOT EXISTS `libseq` (
  `BookId` int(11) NOT NULL,
  `SeqId` int(11) NOT NULL,
  `SeqNumb` tinyint(4) NOT NULL,
  `Level` tinyint(4) NOT NULL default '0',
  PRIMARY KEY  (`BookId`,`SeqId`)
) DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE IF NOT EXISTS `libstat` (
  `BookId` int(11) NOT NULL,
  `Mes` char(4) NOT NULL,
  `N` int(11) NOT NULL,
  UNIQUE KEY `BookId` (`BookId`,`Mes`)
) DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE IF NOT EXISTS `libuseropt` (
  `User` int(11) NOT NULL default '0',
  `Opt` char(1) NOT NULL,
  `Value` char(4) NOT NULL,
  PRIMARY KEY  (`User`,`Opt`),
  KEY `User` (`User`)
) DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE IF NOT EXISTS `libanode` (
  `nid` int(10) unsigned NOT NULL,
  `AvtorId` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`nid`,`AvtorId`)
) DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE IF NOT EXISTS `libbnode` (
  `nid` int(10) unsigned NOT NULL,
  `BookId` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`nid`,`BookId`)
) DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE IF NOT EXISTS `libsrclang` (
  `BookId` int(11) NOT NULL,
  `SrcLang` char(2) NOT NULL,
  PRIMARY KEY  (`BookId`),
  KEY `SrcLang` (`SrcLang`)
) DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE IF NOT EXISTS `libblocked` (
  `BookId` int(11) NOT NULL,
  `uid` int(11) NOT NULL,
  `block` char(1) NOT NULL,
  PRIMARY KEY  (`BookId`)
) DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE IF NOT EXISTS `libpolkarate` (
  `Id` int(11) NOT NULL COMMENT 'libpolka.id',
  `uid` int(11) NOT NULL,
  `rate` tinyint(4) NOT NULL,
  PRIMARY KEY  (`Id`,`uid`),
  KEY `rate` (`rate`)
) DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE IF NOT EXISTS `libtopadvert` (
  `BookId` int(11) NOT NULL,
  `Time` timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  PRIMARY KEY  (`BookId`),
  KEY `Time` (`Time`)
) DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE IF NOT EXISTS `libuserblackgenre` (
`UserId` INT NOT NULL ,
`GenreId` INT NOT NULL ,
PRIMARY KEY ( `UserId` , `GenreId` )
) DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE IF NOT EXISTS `librusec` (
  `cid` varchar(255) NOT NULL default '',
  `data` varchar(21000) default NULL,
  `expire` int(11) NOT NULL default '0',
  `created` int(11) NOT NULL default '0',
  `headers` varchar(255) default NULL,
  `serialized` smallint(6) NOT NULL default '0',
  PRIMARY KEY  (`cid`),
  KEY `expire` (`expire`)
) DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
