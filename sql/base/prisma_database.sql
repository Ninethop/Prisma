/*
SQLyog Community v13.1.9 (64 bit)
MySQL - 8.0.28 : Database - prisma
*********************************************************************
*/

/*!40101 SET NAMES utf8 */;

/*!40101 SET SQL_MODE=''*/;

/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
CREATE DATABASE /*!32312 IF NOT EXISTS*/`prisma` /*!40100 DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci */ /*!80016 DEFAULT ENCRYPTION='N' */;

USE `prisma`;

/*Table structure for table `player_prisma` */

DROP TABLE IF EXISTS `player_prisma`;

CREATE TABLE `player_prisma` (
  `Guid` int NOT NULL,
  `Prisma_1` int unsigned NOT NULL DEFAULT '0',
  `Prisma_2` int unsigned NOT NULL DEFAULT '0',
  `Prisma_3` int unsigned NOT NULL DEFAULT '0',
  `Prisma_4` int unsigned NOT NULL DEFAULT '0',
  `Prisma_5` int unsigned NOT NULL DEFAULT '0',
  `Prisma_6` int unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

/*Table structure for table `prisma` */

DROP TABLE IF EXISTS `prisma`;

CREATE TABLE `prisma` (
  `Guid` int unsigned NOT NULL DEFAULT '0',
  `ID` int unsigned NOT NULL DEFAULT '0',
  `Level` int unsigned NOT NULL DEFAULT '1',
  `Experience` int unsigned NOT NULL DEFAULT '0',
  `Item` int NOT NULL DEFAULT '-1',
  `IV_Stamina` int NOT NULL DEFAULT '-1',
  `IV_Attack` int NOT NULL DEFAULT '-1',
  `IV_Defense` int NOT NULL DEFAULT '-1',
  `IV_SpecialAttack` int NOT NULL DEFAULT '-1',
  `IV_SpecialDefense` int NOT NULL DEFAULT '-1',
  `IV_Speed` int NOT NULL DEFAULT '-1',
  `EV_Stamina` int unsigned NOT NULL DEFAULT '0',
  `EV_Attack` int unsigned NOT NULL DEFAULT '0',
  `EV_Defense` int unsigned NOT NULL DEFAULT '0',
  `EV_SpecialAttack` int unsigned NOT NULL DEFAULT '0',
  `EV_SpecialDefense` int unsigned NOT NULL DEFAULT '0',
  `EV_Speed` int unsigned NOT NULL DEFAULT '0',
  `Spell_0` int NOT NULL DEFAULT '-1',
  `Spell_1` int NOT NULL DEFAULT '-1',
  `Spell_2` int NOT NULL DEFAULT '-1',
  `Spell_3` int NOT NULL DEFAULT '-1',
  UNIQUE KEY `Guid` (`Guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

/*Table structure for table `prisma_template` */

DROP TABLE IF EXISTS `prisma_template`;

CREATE TABLE `prisma_template` (
  `ID` int unsigned NOT NULL AUTO_INCREMENT,
  `Name` mediumtext COLLATE utf8mb4_general_ci NOT NULL DEFAULT '',
  `Description` mediumtext COLLATE utf8mb4_general_ci NOT NULL DEFAULT '',
  `DisplayID` int NOT NULL DEFAULT '-1',
  `Scale` float NOT NULL DEFAULT '1',
  `EvolveLevel` int NOT NULL DEFAULT '-1',
  `EvolveID` int NOT NULL DEFAULT '-1',
  `Type` int NOT NULL DEFAULT '0',
  `BaseExperience` int NOT NULL DEFAULT '0',
  `Stamina` int NOT NULL DEFAULT '0',
  `Attack` int NOT NULL DEFAULT '0',
  `Defense` int NOT NULL DEFAULT '0',
  `SpecialAttack` int NOT NULL DEFAULT '0',
  `SpecialDefense` int NOT NULL DEFAULT '0',
  `Speed` int NOT NULL DEFAULT '0',
  `EVStamina` int NOT NULL DEFAULT '0',
  `EVAttack` int NOT NULL DEFAULT '0',
  `EVDefense` int NOT NULL DEFAULT '0',
  `EVSpecialAttack` int NOT NULL DEFAULT '0',
  `EVSpecialDefense` int NOT NULL DEFAULT '0',
  `EVSpeed` int NOT NULL DEFAULT '0',
  KEY `ID` (`ID`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

/*Data for the table `prisma_template` */

insert  into `prisma_template`(`ID`,`Name`,`Description`,`DisplayID`,`Scale`,`EvolveLevel`,`EvolveID`,`Type`,`BaseExperience`,`Stamina`,`Attack`,`Defense`,`SpecialAttack`,`SpecialDefense`,`Speed`,`EVStamina`,`EVAttack`,`EVDefense`,`EVSpecialAttack`,`EVSpecialDefense`,`EVSpeed`) values 
(1,'Pikachu','',31049,1,-1,-1,0,64,5,8,3,10,4,7,0,1,0,0,0,1);

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;
