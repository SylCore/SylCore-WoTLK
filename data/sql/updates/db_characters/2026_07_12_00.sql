-- DB update 2025_02_23_00 -> 2026_07_12_00
--
DROP TABLE IF EXISTS `character_travel_stats`;
CREATE TABLE `character_travel_stats` (
	`guid` INT UNSIGNED NOT NULL DEFAULT '0' COMMENT 'Global Unique Identifier',
	`walked` BIGINT UNSIGNED NOT NULL DEFAULT '0' COMMENT 'Counts as both walking + running',
	`mounted` BIGINT UNSIGNED NOT NULL DEFAULT '0' COMMENT 'Mounted',
	`swimming` BIGINT UNSIGNED NOT NULL DEFAULT '0' COMMENT 'Swimming',
	`flying` BIGINT UNSIGNED NOT NULL DEFAULT '0' COMMENT 'Flying',
	PRIMARY KEY (`guid`) USING BTREE
)
COMMENT='Player System (Custom SylCore System For Tracking Movement Distance)'
COLLATE='utf8mb4_unicode_ci'
ENGINE=InnoDB;
