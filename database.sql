-- phpMyAdmin SQL Dump
-- version 4.5.4.1deb2ubuntu1
-- http://www.phpmyadmin.net
--
-- Хост: localhost
-- Время создания: Май 15 2016 г., 17:24
-- Версия сервера: 5.7.12-0ubuntu1
-- Версия PHP: 7.0.4-7ubuntu2

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- База данных: `s1user2`
--

-- --------------------------------------------------------

--
-- Структура таблицы `admlogs`
--

CREATE TABLE `admlogs` (
  `id` int(11) NOT NULL,
  `admlogin` varchar(40) CHARACTER SET utf8 NOT NULL,
  `action` varchar(32) CHARACTER SET utf8 NOT NULL,
  `data` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Структура таблицы `banlist`
--

CREATE TABLE `banlist` (
  `id` int(11) NOT NULL,
  `userid` int(11) NOT NULL,
  `reacon` varchar(60) CHARACTER SET utf8 NOT NULL,
  `data` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `permanet` tinyint(1) NOT NULL DEFAULT '1',
  `unbandata` timestamp NULL DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Структура таблицы `cron`
--

CREATE TABLE `cron` (
  `id` int(11) NOT NULL,
  `name` varchar(20) COLLATE utf8_unicode_ci NOT NULL,
  `value` int(10) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

--
-- Дамп данных таблицы `cron`
--

INSERT INTO `cron` (`id`, `name`, `value`) VALUES
(1, 'CleanNeactiveUsers', 0),
(2, 'OptimizeBD', 16);

-- --------------------------------------------------------

--
-- Структура таблицы `passer`
--

CREATE TABLE `passer` (
  `id` int(255) NOT NULL,
  `id_login` int(255) NOT NULL,
  `date` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `passer` varchar(10) COLLATE utf8_unicode_ci NOT NULL,
  `email` varchar(30) COLLATE utf8_unicode_ci NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Структура таблицы `private`
--

CREATE TABLE `private` (
  `id` int(255) NOT NULL,
  `idR` int(255) NOT NULL,
  `type` varchar(2) NOT NULL,
  `textMessage` text NOT NULL,
  `idOt` int(255) NOT NULL,
  `dateMessage` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `commandMessage` varchar(255) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Структура таблицы `regser`
--

CREATE TABLE `regser` (
  `id` int(255) NOT NULL,
  `date` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `regser` varchar(10) COLLATE utf8_unicode_ci NOT NULL,
  `email` varchar(30) COLLATE utf8_unicode_ci NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Структура таблицы `rooms`
--

CREATE TABLE `rooms` (
  `id` int(255) NOT NULL,
  `nameTextRoom` text COLLATE utf8_unicode_ci NOT NULL,
  `dateCreatRoom` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `idUserCreat` varchar(255) COLLATE utf8_unicode_ci NOT NULL,
  `idsUsers` varchar(255) COLLATE utf8_unicode_ci NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

--
-- Дамп данных таблицы `rooms`
--

INSERT INTO `rooms` (`id`, `nameTextRoom`, `dateCreatRoom`, `idUserCreat`, `idsUsers`) VALUES
(2, 'Случай', '2015-10-25 13:43:25', '11', '12, 13'),
(1, 'Регион Кластеров', '2015-10-25 12:59:57', '10', '11, 12, 13'),
(3, 'Завалинка', '2015-10-25 12:40:49', '10', '11, 12, 13'),
(105, 'Клюква', '2015-10-25 12:40:49', '10', '11, 12, 13'),
(106, 'Сквозняк', '2015-10-25 12:40:49', '10', '11, 12, 13'),
(108, 'TestRoom', '2016-04-07 17:00:00', '11', '');

-- --------------------------------------------------------

--
-- Структура таблицы `unigies`
--

CREATE TABLE `unigies` (
  `id` int(11) NOT NULL,
  `unigie` varchar(255) NOT NULL,
  `ip` varchar(60) NOT NULL,
  `cluObject` varchar(255) NOT NULL,
  `cookie` varchar(255) NOT NULL,
  `init` varchar(255) NOT NULL,
  `initV` varchar(255) NOT NULL,
  `date` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `lastDate` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  `idUser` int(255) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Структура таблицы `users`
--

CREATE TABLE `users` (
  `id` int(255) NOT NULL,
  `podtverdit` int(1) NOT NULL DEFAULT '0',
  `last_IP` varchar(30) CHARACTER SET utf8 COLLATE utf8_unicode_ci DEFAULT NULL,
  `IP_REG` varchar(30) CHARACTER SET utf8 COLLATE utf8_unicode_ci DEFAULT NULL,
  `group` varchar(255) NOT NULL DEFAULT 'acc',
  `init` varchar(20) CHARACTER SET cp1251 COLLATE cp1251_bin NOT NULL DEFAULT 'None',
  `initV` varchar(15) NOT NULL DEFAULT '0.0',
  `name` varchar(20) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `pass` varchar(40) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `timeZone` varchar(20) NOT NULL,
  `real_name` varchar(20) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `data` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `last_online` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `status` varchar(255) DEFAULT NULL,
  `friends` varchar(255) DEFAULT NULL,
  `email` varchar(100) NOT NULL,
  `prefix` varchar(255) DEFAULT NULL,
  `HAID` int(11) NOT NULL DEFAULT '0',
  `colored` varchar(12) NOT NULL DEFAULT '00F',
  `hidden` tinyint(1) NOT NULL DEFAULT '0',
  `baned` int(11) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Дамп данных таблицы `users`
--

INSERT INTO `users` (`id`, `podtverdit`, `last_IP`, `IP_REG`, `group`, `init`, `initV`, `name`, `pass`, `timeZone`, `real_name`, `data`, `last_online`, `status`, `friends`, `email`, `prefix`, `HAID`, `colored`, `hidden`, `baned`) VALUES
(11, 1, '178.187.181.136', '178.125.125.154', 'acc, ADM, SUPERUSER, MODER', 'Qt', '1.0.5', 'Gravit', 'f94e044b2f8dfa3c9b603bfb3ce61d7a', 'Asia/Dhaka', 'Александр Сальков', '2016-05-15 10:23:57', '2016-01-15 21:56:51', '', '', 'xellgf2013@ya.ru', '', 0, '00f', 0, 0),
(10, 1, '178.125.217.53', '178.125.000.000', 'acc, ADM, SUPERUSER, MODER', 'JarConsole', 'cluChat_0.0.1', 'Linux', '6ebe76c9fb411be97b3b0d48b791a7c9', 'Europe/Minsk', 'Котляров Денис', '2016-05-15 10:23:51', '2016-01-16 02:08:47', 'Сплю', '1, 4', 'zyvak_c_neba@mail.ru', '47', 0, '00F', 0, 0),
(54, 1, NULL, NULL, 'acc, ADM, SUPERUSER, MODER', 'Console', '1.0', 'root', '', 'Moscow', 'Администратор', '2016-05-15 09:21:48', '2016-05-15 08:49:44', NULL, NULL, 'admin@clustergroup.ml', NULL, 0, '00F', 1, 0);

--
-- Индексы сохранённых таблиц
--

--
-- Индексы таблицы `admlogs`
--
ALTER TABLE `admlogs`
  ADD PRIMARY KEY (`id`);

--
-- Индексы таблицы `banlist`
--
ALTER TABLE `banlist`
  ADD PRIMARY KEY (`id`);

--
-- Индексы таблицы `cron`
--
ALTER TABLE `cron`
  ADD PRIMARY KEY (`id`);

--
-- Индексы таблицы `passer`
--
ALTER TABLE `passer`
  ADD PRIMARY KEY (`id`);

--
-- Индексы таблицы `private`
--
ALTER TABLE `private`
  ADD PRIMARY KEY (`id`);

--
-- Индексы таблицы `regser`
--
ALTER TABLE `regser`
  ADD PRIMARY KEY (`id`);

--
-- Индексы таблицы `rooms`
--
ALTER TABLE `rooms`
  ADD PRIMARY KEY (`id`);

--
-- Индексы таблицы `unigies`
--
ALTER TABLE `unigies`
  ADD PRIMARY KEY (`id`);

--
-- Индексы таблицы `users`
--
ALTER TABLE `users`
  ADD PRIMARY KEY (`id`);

--
-- AUTO_INCREMENT для сохранённых таблиц
--

--
-- AUTO_INCREMENT для таблицы `admlogs`
--
ALTER TABLE `admlogs`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT для таблицы `banlist`
--
ALTER TABLE `banlist`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT для таблицы `cron`
--
ALTER TABLE `cron`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=3;
--
-- AUTO_INCREMENT для таблицы `passer`
--
ALTER TABLE `passer`
  MODIFY `id` int(255) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT для таблицы `private`
--
ALTER TABLE `private`
  MODIFY `id` int(255) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT для таблицы `regser`
--
ALTER TABLE `regser`
  MODIFY `id` int(255) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=46;
--
-- AUTO_INCREMENT для таблицы `rooms`
--
ALTER TABLE `rooms`
  MODIFY `id` int(255) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=109;
--
-- AUTO_INCREMENT для таблицы `unigies`
--
ALTER TABLE `unigies`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT для таблицы `users`
--
ALTER TABLE `users`
  MODIFY `id` int(255) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=55;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
