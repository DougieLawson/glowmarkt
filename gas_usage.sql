--
-- Copyright © Dougie Lawson, 2021, All rights reserved 
--

CREATE TABLE `gas_usage` (
  `datetime` timestamp NOT NULL DEFAULT current_timestamp() ON UPDATE current_timestamp(),
  `reading` float(12,3) DEFAULT NULL,
  `daily` float(12,3) DEFAULT NULL,
  `weekly` float(12,3) DEFAULT NULL,
  `monthly` float(12,3) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;



CREATE TABLE `gas_reading` (
  `current_reading` float(12,3) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

