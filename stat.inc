<?php
module_load_include('inc', 'librusec');

function libStat($f='', $f2 = '') {
  Global $user;
  $admin = user_access('библиотекарь');

  $head = "<a href=/stat/about>Как устроена библиотека</a><br>Рейтинги: <a href=/stat/24>Топ дня</a> &nbsp; 
<a href=/stat/w>Топ недели</a> &nbsp; 
<a href=/stat/b>Популярные книги</a> &nbsp 
<a href=/stat/r>Оценённые книги</a> &nbsp 
<a href=/stat/a>Популярные авторы</a> &nbsp 
<a href=/stat/plo>Плодовитые авторы</a> <br>
Библиотечное: <a href=/stat/my>Моя работа</a> &nbsp; 
<a href=/stat/document-author>Авторы файлов</a> &nbsp; 
<a href=/stat/u>Статистика добавлений</a> &nbsp 
<a href=/stat/invalid>Невалидные файлы</a> &nbsp 
<a href=/stat/tot>Расклад по типам файлов</a>
<br>";

  if ($user->uid > 0) $head .= 
"Общая статистика сайта: <b>2009</b>: <a href=/awstats/>AWStats 2009</a>
<a href=/awstats/2009.08.html>2009.08</a> <a href=/awstats/2009.07.html>2009.07</a>
<a href=/awstats/2009.06.html>2009.06</a> <a href=/awstats/2009.05.html>2009.05</a> <a href=/awstats/2009.04.html>2009.04</a> 
<a href=/awstats/2009.03.html>2009.03</a> <a href=/awstats/2009.02.html>2009.02</a> <a href=/awstats/2009.01.html>2009.01</a><br>
<b>2008</b>: <a href=/awstats/2008.12.html>2008.12</a> <a href=/awstats/2008.11.html>2008.11</a> <a href=/awstats/2008.10.html>2008.10</a> 
<a href=/awstats/2008.09.html>2008.09</a> <a href=/awstats/2008.08.html>2008.08</a> <a href=/awstats/2008.07.html>2008.07</a> 
<a href=/awstats/2008.06.html>2008.06</a> <a href=/awstats/2008.05.html>2008.05</a> <a href=/awstats/2008.04.html>2008.04</a> 
<a href=/awstats/2008.03.html>2008.03</a> <a href=/awstats/2008.02.html>2008.02</a> <a href=/awstats/2008.01.html>2008.01</a><br>";

  $admin_actions = array (
    'edit' => 'Вся деятельность пользователей', 
    'edit/nobibl' => 'Деятельность не библиотекарей', 
    'edit/delete' => 'Удаления книг', 
    'uploaders' => 'Топ 100 заливщиков', 
    'akt' => 'Активные', 
    'bibl' => 'Библиотекари', 
    'block' => 'Враги', 
    'q' => 'Оценки файлов',
    'litres' => 'Книги заблокированные Литрес'
  );
  if ($admin) {
    $head .= "<br><h4>Библиотекарское</h4>";
    foreach($admin_actions as $akt => $name)
      $head .= "<a href=/stat/$akt>$name</a> &nbsp; ";
    $head .= "<br><hr>";
  } elseif ($admin_actions[$f]) {
    return "Только админам";
  } 
  $head .= '<br>';
  $titles = array(
    'about' => 'Про Либрусек', 
    'w'     => 'Топ 100 за последние 7 дней (по скачиваниям)',
    '24'    => 'Топ 100 за последние cутки (по скачиваниям)', 
    'tot'   => 'Наличие книг в библиотеке', 
    'b'     => 'Популярные книги', 
    'a'     => 'Популярные авторы', 
    'plo'   => 'Плодовитые авторы', 
    'r'     => 'Книги, получившие максимальное количество оценок',
    'lang'  => 'Языки',
    'type'  => 'Типы',
    'q'     => 'Оценки файлов',
    'akt'   => 'Список пользователей изменяющих библиотеку',
    'invalid'=> 'Негодные FB2',
    'u'	     => 'Пополнение',
  );
  set_title($titles[$f]);
// статистика хранится в кэше, пересчитывается раз в 170 минут, если позволит загрузка сервера.

  if ($f == 'w' || $f == 'b' || $f == '24' || $f == 'r') 
    if ($gg = libListBlackGG()) 
      $BGLIST = "JOIN libgenre USING(BookId) WHERE NOT EXISTS(SELECT UserId FROM libuserblackgenre bg WHERE UserId = $user->uid AND libgenre.GenreId = bg.GenreId)";
    elseif(!$user->uid)  
      $BGLIST = "JOIN libbook USING(BookId) WHERE NOT libbook.Blocked";
  if (!$admin_actions[$f] && !$admin && $f != 'my') {
    $cid = "libstat-$f-$f2-".$_GET['page'];
    if ($BGLIST) $cid .= "g$gg";
    if ($r = libcg($cid, 100000)) return $head.$r;
    $key = sem_get(13, 30);
    if (!sem_acquire($key)) return;
    if ($r = libcg($cid, 100000)) {
      sem_release($key);
      return $head.$r;
    }
  }  
  
  switch ($f) {
    case 'about':
      $r = "<p>Основа - свободная CMS Drupal версия 6.9(<a href=http://drupal.org>http://drupal.org</a>) + русификация интерфейса (<a href=http://drupal.ru>http://drupal.ru</a>) + <a href=/allbooks>модуль Либрусек.</a>.<ol>Установленные модули Друпала:";
      $sth = SELECT ("name FROM `system` WHERE `type` LIKE 'module' AND `status` = 1 ORDER BY 1");
       while ($a1 = dbf($sth)) $r .= "<li>".$a1->name;
      $r .= "</ol>\n
<p>Тема BlueBreeze (<a href=http://drupal.org/project/bluebreeze>http://drupal.org/project/bluebreeze</a>)
<p><p>Начальное наполнение - SharkLib, криво переделанная из dos-txt в win-fb2
<p>Текущие дополнения закачиваются пользователями. Начали эти добрые люди с того, что залили архивы альдебарана и фикшена целиком. Дубли между SharkLib и указанным пока не довыловлены. Но работа ведётся, в основном силами пользователей же.
";  
    break;

    case 'w':
      $sth = SELECT ("SUM(1) as Num, BookId FROM liblog as l $BGLIST GROUP BY 2 ORDER BY 1 DESC LIMIT 200");
      while ($a1=dbf($sth)) {
        $b = $a1->BookId;
        $n = $a1->Num + Sel("SUM(1) FROM libreaded WHERE BookId = $b");
        $gb = Sel("GoodId FROM libjoinedbooks WHERE BadId = $b");
        if ($gb) $READED[$gb] += $n;
        else $READED[$b] += $n;
      }
      arsort($READED);  
      foreach($READED as $b=>$n) {
        if ($i++>100) break;
        $av = libAvtorsNames($b);
        $r .= "<li>$av - ".bl($b);
      }
      $r = libListBlackGenre()."<ul>$r</ul>";
    break;

    case '24': 
      if ($BGLIST) $BGLIST .= " AND ";
      else $BGLIST = "WHERE";
      $sth = SELECT("SUM(1) as Num, liblog.BookId as BookId FROM `liblog` JOIN libbook USING (BookId) $BGLIST liblog.Time > SUBDATE(NOW( ), 1) GROUP BY 2 ORDER BY 1 DESC LIMIT 100");
      while ($a1=dbf($sth)) {  
        $av = libAvtorsNames($a1->BookId);
        $r .= "<li>$av - ".bl($a1->BookId);
      }
      $r = libListBlackGenre()."<ul>$r</ul>";
    break;
    
    case 'b':
      if ($BGLIST) {
        $BGLIST .= " AND ";
        $GROUP = "GROUP BY 1, 2";
      } else {
        $BGLIST = "WHERE";
      }
      $q = "SELECT N, BookId FROM libbook $BGLIST NOT Deleted&1 $GROUP ORDER BY 1 DESC";
      $count = "SELECT count(*) FROM libbook $BGLIST NOT Deleted&1";
      $sth = pager_query($q, 100, 0, $count);
      while ($a1=dbf($sth)) {  
        $av = libAvtorsNames($a1->BookId);
        $r .= "<li>$av - ".bl($a1->BookId);
      }
      $r = libListBlackGenre()."<ul>$r</ul>";
    break;

    case 'u':
      $sth = SELECT("SUM(1) AS N, substr(Time, 1,7) AS Mes FROM `libbook` GROUP BY 2");
      while ($a1=dbf($sth)) $r .= "<tr><td>$a1->Mes<td>$a1->N";
      $r = "<table>$r</table>";
    break;
    case 'tot': 
      $r =  "<h4>Всего</h4>".Sel ("Count(*) FROM libbook WHERE NOT (Deleted&1)");
      $r .= "<table><tr><th>По типам файлов<th>По языкам<tr><td>";
      $sth = SELECT ("sum(1) as s, FileType FROM libbook WHERE NOT (Deleted&1) GROUP BY 2 ORDER BY 1 DESC");
      while ($a1=dbf($sth))   
        $r .= "<a href='/stat/type/$a1->FileType'>$a1->FileType: $a1->s</a><br>";
      $r .= "<td valign=top>";
      $sth = SELECT ("sum(1) as s, Lang FROM libbook WHERE NOT (Deleted&1) GROUP BY 2 ORDER BY 1 DESC");
      while ($a1=dbf($sth))   
        $r .= "<a href='/stat/lang/$a1->Lang'>$a1->Lang: $a1->s</a><br>";
      $r .= "</table>";
    break;

    case 'r': 
      $order = "ORDER BY 1 DESC";
      if ($f2 == 'av') $order = "ORDER BY av DESC, 1 DESC";
      if ($f2 == 'g') $order = "ORDER BY m1 DESC, 1 DESC";
      if ($f2 == 'b') $order = "ORDER BY m2, 1 DESC";
      $sth = pager_query("SELECT SUM(1) as s, SUM(Rate) as r, MAX(Rate) AS m1, MIN(Rate) AS m2, BookId as b, AVG(Rate) as av FROM librate $BGLIST GROUP BY BookId $order", 100, 0, 
                         "SELECT count(*) FROM libbook $BGLIST");
      $r = libListBlackGenre()."<table><tr><th>Книга<th><a href=/stat/r>Оценивших</a><th><a href=/stat/r/av>Среднее</a><th><a href=/stat/r/g>Лучшее</a><th><a href=/stat/r/b>Худшее</a>";
      while ($a1=dbf($sth)) 
        $r .= "<tr><td>".DoLibRow($a1->b, 'nobreak author')."<td>$a1->s<td>".(((integer)(100*$a1->r/$a1->s))/100)."<td>$a1->m1<td>$a1->m2\n";
      $r .= "</table>";
    break;

    case 'q': 
      for ($i = 1; $i <= 5; $i++) $r .= "<a href=/stat/q/q$i>Оценено на $i</a> &nbsp; ";
      $r .= '<br>';
      $b = arg(2);
      if ($b == 'user') {
        $u = 1*arg(3);
        if (!$u) break;
        $r .= "<h3>Оценки файлов, проставленные товарищем ".ul($u)."</h3>";
        $st = pgq("SELECT BookId, q FROM libquality WHERE uid = $u", Sel("count(*) FROM libquality WHERE uid = $u"));
      } elseif ($b > 0) {
        $r .= "<h3>Оценки файла ".bl($b)."</h3>";
        $st = pgq("SELECT BookId, uid, q, name FROM libquality JOIN users USING(uid) WHERE BookId = $b", Sel("count(*) FROM libquality WHERE BookId = $b"));
      } elseif(substr($b,0,1) == 'q') {
        $q = substr($b,1,1);
        $r .= "<h3>Файлы, оценённые на $q</h3>";
        $st = pgq("SELECT BookId, uid, q, name FROM libquality JOIN users USING(uid) WHERE q = $q", 
                Sel("count(*) FROM libquality WHERE q = $q"));
      } else {
        $st = pgq("SELECT BookId, ROUND(AVG(q)) AS q, SUM(1) AS n FROM libquality GROUP BY 1", Sel("count(*) FROM libquality"));
        $u = 1;
      }  
      $r .= "\n<table>";
      if (!$u) $r .= "<th>Оценщик";
      $r .= "<th>Файл";
      if (!$b) $r .= "<th>Количество оценок<th>Средний балл\n";
      else $r .= "<th>Оценка\n";
      while ($a1 = dbf($st)) {
        $r .= "<tr>";
        if (!$u) $r .= "<td> <a href=/stat/q/user/$a1->uid>$a1->name</a> ";
        $r .= "<td> ".bl($a1->BookId);
        if ($a1->n) $r .= " <td><a href=/stat/q/$a1->BookId> $a1->n </a>";
        $r .= " <td> $a1->q";
      }
      $r .= "\n</table>\n";
    break;  

    case 'lang':
      $la = arg(2);  
      $st = pgq("SELECT BookId FROM libbook WHERE Lang = '$la' AND NOT (Deleted&1)", 
                Sel ("count(BookId) FROM libbook WHERE Lang = '$la' AND NOT (Deleted&1)"));
      while ($a1=dbf($st)) $r .= DoLibRow($a1->BookId);
    break;

    case 'type':
      $tp = arg(2);  
      $st = pgq("SELECT BookId FROM libbook WHERE FileType = '$tp' AND NOT (Deleted&1)", 
                 Sel ("count(BookId) FROM libbook WHERE FileType = '$tp' AND NOT (Deleted&1)"));
      while ($a1=dbf($st)) $r .= DoLibRow($a1->BookId);
    break;  

    case 'litres':
      $st = pgq("SELECT BookId FROM libbook WHERE Blocked AND NOT (Deleted&1) ORDER BY BookId DESC", 
           Sel ("count(BookId) FROM libbook WHERE Blocked AND NOT (Deleted&1)"));
      while ($a1=dbf($st)) $r .= DoLibRow($a1->BookId);
    break;  

    case 'a': 
       $r = "<ul>";
//     $n = Sel ("count (DISTINCT AvtorId) FROM liblog JOIN libavtor USING (BookId)") - 2;
       $n = 20000;
       $sth = pgq ("SELECT SUM(N) as N, AvtorId FROM `libbook` JOIN libavtor USING (BookId) GROUP BY AvtorId ORDER BY 1 DESC", $n);
       while ($a1=dbf($sth)) $r .=  "<li>".avl($a1->AvtorId);
       $r .= "</ul>";
    break;

    case 'plo':
      $sth = pgq("SELECT SUM(1) as s, AvtorId FROM `libavtor` JOIN libbook USING(BookId) WHERE NOT (Deleted&1) GROUP BY 2 ORDER BY 1 DESC", 10000);
      $r = "<ul>";
      while ($a1=dbf($sth)) $r .= "<li>$a1->s: ".avl($a1->AvtorId); 
      $r .= "</ul>";
    break;

    case 'undouser': 
      $st = SELECT ("* FROM libactions WHERE UserName = '".arg(2)."' ORDER BY ActionID DESC");
      while ($a1 = dbf($st)) $r .= $a1->ActionUndo.";<br>";
    return $head.$r;  
      
    case 'blockuser': 
      $u = (int)arg(2);
      $u = Sel ("uid FROM users WHERE uid = $u");
      if (!($u > 0)) return "Ошибка"; 
      $uname = Sel ("name FROM users WHERE uid = $u");
      INSERT(users_roles, "uid, rid", "$u, 4");
      LogAction("INSERT INTO users_roles (uid, rid) VALUES ($u, 4)", 
                "Заблокировал пользователя $uname ($u)", 
                "DELETE FROM users_roles WHERE uid=$u AND rid=4");
      $r = "Пользователь <a href=/user/$u>$uname</a> заблокирован.";
    return $head.$r;  

    case 'edit': 
      $users = Array();
      $r .= '<script>function statshowuser() {document.location="/stat/edit/"+document.getElementById("statuser").value}</script>';
      $r .= "<select id=statuser onChange=statshowuser()>";
      $r .= "<option>";
      $s = SELECT ("DISTINCT UserName FROM libactions");
      while ($a1 = dbf($s)) 
        if ($a1->UserName) 
          $r .= '<option value="'.urlencode($a1->UserName)."\">$a1->UserName";
      $r .= '</select>';
      case 'my':
        if ($f == 'my') {
          global $user;
          if (!$user->uid) return $head."Залогиньтесь, будьте так добры";
          $u = $user->name;
        } else {
          $u = urldecode(arg(2));
        }
      if ($u) {
        if ($u == 'delete') {
          $w = "WHERE ActionDesc like 'Delete Book%'";
        } elseif ($u == 'nomod' || $u == 'nobibl') { 
          $w = "JOIN users ON(UserName=name) LEFT JOIN users_roles USING(uid) WHERE isnull(rid)";
        } elseif ($u == 'b' && arg(3)) {
          $w = "WHERE BookId = ".(1*arg(3));
          $r .= "<h3>История книги ".bl(arg(3))."</h3>";
        } else {
          $uid = Sel("uid FROM users WHERE name = \"$u\"");
          $r .= "<h3>Деятельность товарища <a href=/user/$uid>$u</a></h3>";
          if (SuperUser()) $r .= "<a href=\"/stat/undouser/$u\">Откатить работу $u</a>"; 
          if ($f == 'edit')
          $r .= " &nbsp <a href=\"javascript:cnf('Вы уверены в необходимости блокирования пользователя $u?', '/stat/blockuser/$uid')\">".
                "Заблокировать пользователя $u</a> &nbsp <a href=/stat/edit/delete>Удаления</a>";
          $w = "WHERE UserName = '$u'";
          $uu = 1;
        }
      }  
      $N = Sel ("count(*) FROM libactions $w");
      $r .= " Всего - $N актов изменения базы";
 
      $st = pgq("SELECT * FROM libactions $w ORDER BY ActionID DESC", $N);
      $r .= "<table border>";
      while ($a1 = dbf($st)) {
        $d = $a1->ActionDesc;
        if (strstr($d, "Copy Sequense info from")) continue;
        $r .= "<tr>";
        if (!$uu) $r .= '<td><a href="/stat/edit/'.urlencode($a1->UserName)."\">$a1->UserName</a>";
        if ($a1->UserName === 'cotopaxi' || $a1->UserName === 'golma1' || $a1->UserName === 'izaraya') $A = 'а'; else $A = '';
        $d = preg_replace ('/Avtor Alias (\d+) to (\d+)/ie', "'Синоним '.avl('\\1').' =&gt; '.avl('\\2')", $d);
        $d = preg_replace ('/Change Avtor (\d+) to (\d+) for/ie', "'Заменил$A '.avl('\\1').' =&gt; '.avl('\\2').' у книги'", $d);
        $d = preg_replace ('/Delete Avtor (\d+) from Book (\d+)/ie', "'Убрал$A из '.bl('\\2').' автора '.avl('\\1')", $d);
        $d = preg_replace ('/Add Book (\d+) Avtor (\d+)/ie', "'Добавил$A в '.bl('\\1').' автора '.avl('\\2')", $d);  
        $d = preg_replace ('/Rename avtor (\d+)/ie', "'Поправил$A имя '.avl('\\1') ", $d);
        $d = preg_replace ('/UnDelete Book (\d+)/ie', "'Восстановил$A книгу '.bl('\\1')", $d);
        $d = preg_replace ('/Delete Book (\d+)/ie', "'Удалил$A книгу '.bl('\\1')", $d);
        $d = preg_replace ('/Avtor (\d+)/ie', " avl('\\1') ", $d);
        $d = preg_replace ('/Book (\d+)/ie', " bl('\\1') ", $d);
        $d = preg_replace ('/Delete Seq (\d+) from/', "Убрал$A сериал у книги ", $d);
        $d = preg_replace ('/Change Title (\d+) from (.+) to (.+)/e', "'Переименовал$A книгу '.bl('\\1').' из \\2'", $d);
        $d = preg_replace ('/Del genre (\d+) from/e', "'Убрал$A жанр '.Sel('GenreCode FROM libgenrelist WHERE GenreId = \\1').' из \\2'", $d);
        $d = preg_replace ('/Add genre (\d+) to/e', "'Добавил$A жанр '.Sel('GenreCode FROM libgenrelist WHERE GenreId = \\1').' в \\2'", $d);
        $d = preg_replace ('/Join books (\d+) (\d+)/ie', "'Объединил$A книги '.bl('\\1').' и '.bl('\\2')", $d);
        $d = preg_replace ('/Set SeqNumb to (\d+)/', "Прописал$A номер серии у книги ".bl($a1->BookId), $d);
        if ($d === 'Add book') {
          $bi = $a1->BookId;
          $d =  "Добавил$A книгу ".($bi?bl($bi):' - безуспешно');
        }
        $SQL = preg_replace('/[\'\"]/', '', $a1->ActionSQL);
        $tm = preg_replace('/ /', '&nbsp;', $a1->Time);
        $r .= "<td>$tm<td>$d<td><a href=\"javascript:alert('$SQL')\">SQL</a>";
      }
      $r .= "</table>";
    break;

    case 'uploaders':
      $st = SELECT("name, SUM(1) AS n, uid FROM `libactions` JOIN users ON(name=UserName) WHERE `ActionDesc` = 'Add book' GROUP BY 1 ORDER BY 2 DESC LIMIT 100");
      $r .= "<h3>Сто пользователей, добавивших больше всех книг</h3><table>";
      while ($a1 = dbf($st)) {
        $r .= "<tr><td><a href=/user/$a1->uid>$a1->name</a><td><a href = '/stat/edit/$a1->name'>$a1->n</a><td>";
        if (Sel ( "uid FROM users_roles WHERE uid=$a1->uid AND rid=3")) $r .= "библиотекарь";
      }
      $r .= "</table>";
    break;

    case 'block':$rid = 4;
    case 'bibl':
      if (!$rid) $rid = 3;
      $st = SELECT("name, uid FROM `users_roles` JOIN users USING(uid) WHERE rid = $rid AND uid > 1 GROUP BY 1 ORDER BY name");
      $r .= "<h3>Список ".($rid==4?'врагов':'библиотекарей')."(в алфавитном порядке)</h3><ul>";
      while ($a1 = dbf($st)) $r .= "<li><a href=/user/$a1->uid>$a1->name</a>";
      $r .= "</ul>";
    break;

    case 'akt': 
      $r .= "<ul>";
      $st = SELECT('SUM(1) AS n, UserName, rid FROM libactions JOIN users ON (name=UserName) LEFT JOIN users_roles USING (uid) WHERE rid < 4 OR ISNULL(rid)  GROUP BY 2, 3 ORDER BY 1 DESC LIMIT 200');
      while ($a1 = dbf($st)) {
        $us = urlencode($a1->UserName);
        $r .= "<li>$a1->n <a href='/stat/edit/$us'>$a1->UserName</a>";
        if ($a1->rid == 3) $r .= " (<b>библиотекарь</b>)";
        if ($a1->rid == 4) $r .= " (заблокирован)";
      }
    break;  

    case 'document-author': 
      $a = arg(2);
      if (arg(3)) $a .= '/'.arg(3);
      if ($a) {
        if ($a == 'empty') $a = '';
        else $a = urldecode($a);
        $r .= "<h3>Книги изготовленные товарищем <b>$a</b></h3>";  
        $sth = pgq("SELECT BookId FROM libbook WHERE NOT (Deleted&1) AND FileAuthor = \"$a\" ORDER BY 1", 
                 Sel ("Count(*) FROM libbook WHERE NOT (Deleted&1) AND FileAuthor = \"$a\" "));
        while ($a1 = dbf($sth)) $r .= DoLibRow($a1->BookId, 'edit authors')."<br>";
      } else {
        $sth = SELECT("SUM(1) AS n, FileAuthor FROM libbook WHERE NOT (Deleted&1) GROUP BY 2 ORDER BY 1 DESC");
        while ($a1 = dbf($sth)) {
          $aname = urlencode($a1->FileAuthor);
          if ($aname == '') $aname = 'empty';
          $r .= "<li><a href='/stat/document-author/$aname'>$a1->FileAuthor:$a1->n</a>";
        }  
      }
    return $head.$r.theme('pager');  
    
    case 'invalid':
      $r .= "<h2>Файлы, не прошедшие валидацию</h2><table>";
      $book = new DB;
      foreach (file('badbooks.lst') as $fb2) {
        $b = 1*$fb2; 
        $i++;
        if ($b && !$book->Deleted($b)) {
          $r .= DoLibRow($b, 'authors edit')."<a href=/b/$b/validate>(ошибки)</a>&nbsp;$i";
        }  
      }
      $r .= "</table>";
    break;
    default:
      $a1 = S("count(BookId) AS N, sum(FileSize)/1024000 AS S FROM libbook WHERE NOT (Deleted&1)");
      $size = (integer) ($a1->S/1024);
      $r .= "<p>Всего книг в библиотеке - $a1->N томов, $size гигабайт";
      $r .= "<p>Всего представлено авторов - ".(Sel("count(*) FROM libavtorname") - Sel ("count(*) FROM libavtoraliase"));
      $r .= "<p>Зарегистрированных пользователей - ".Sel("count(*) FROM users");
      $r .= "<p>Посетителей в день - 100K";
      $r .= "<p>Обновлений за последние 30 дней - ".Sel("count(*) FROM libbook WHERE now() - INTERVAL 30 DAY < Time AND NOT (Deleted&1)");
    break;
  }
  if ($key) sem_release($key);
  return $head.libcs($cid, $r.theme('pager'));
}

function libAvtorsNames($b) {
  if (Sel ("count(AvtorId) FROM libavtor WHERE BookId = $b") > 3) return "Сборник";
  return avl(Sel("AvtorId FROM libavtor WHERE BookId = $b"));
}

function libstatblock($delta) {
  if ($r = libcg("block.$delta", 50000)) return $r;
  $sql = array(
    'besta' => 'SUM(N) as N, AvtorId, LastName FROM libbook JOIN libavtor USING (BookId) JOIN libavtorname USING(AvtorId)', 
    'bestb' => 'SUM(N) as N, BookId, Title FROM libbook',
    'new'   => 'BookId, Title FROM libbook',
    'bestw' => 'SUM(1) as N, BookId, Title FROM liblog JOIN libbook USING(BookId) WHERE libbook.N > 1000');
  if (!$sql[$delta]) return '';
  $key = sem_get(12, 30);
  if (!sem_acquire($key)) return;
  if ($r = libcg("block.$delta", 50000)) {
    sem_release($key);
    return $r;
  }  
  $sth = SELECT($sql[$delta]." GROUP BY 2 ORDER BY 1 DESC LIMIT 99");
  while ($a1 = dbf($sth)) 
    if ($a1->BookId) $r .= "<a href=/b/$a1->BookId>$a1->Title</a> ";
    else $r .= "<a href=/a/$a1->AvtorId>$a1->LastName</a> ";
  libcs("block.$delta", $r);
  sem_release($key);
  return $r; 
}

//скачано книжек в день - SELECT SUBSTR(time,1,10) as d, SUM(N) FROM `liblog` group by d
//пожертвований помесячно -  SELECT SUM(Sum)/100, substr(time, 1, 7), type FROM `libdonations` GROUP BY 2,3
//DELETE FROM users WHERE created < UNIX_TIMESTAMP() - 3600*24*7 AND access=0 AND login=0  
//SELECT sum(1), substr(location, 32,2) FROM `watchdog` WHERE type = 'arooga' GROUP BY 2 ORDER BY 1 DESC
