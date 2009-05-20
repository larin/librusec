<?php
include_once drupal_get_path('module', 'librusec') .'/librusec.inc';

function libPolka() {
  Global $user, $ButtonStyle;
  if (arg(1) == 'show' && arg(2) == 'all')
    if (arg(3) == 'rss') return libPolkaShowAllRSS();
    else return libPolkaShow('all');
  $u = $user->uid;
  if (!$u || !user_access('пользоваться полкой')) 
    return "У вас нет доступа к книжной полке. Извините.";

  $r = "<a href=/polka/rated>Оценки</a> &nbsp; <a href=/polka/readed>Прочитано</a> &nbsp; <a href='/polka'>Впечатления</a> &nbsp; <a href=/polka/friends>Единомышленники</a> &nbsp; <a href=/polka/friends/books>Советы единомышленников</a><br><br>";

  switch (arg(1)) {
    case readed:
      return $r.libPolkaReaded($u);
    case rated:
      return $r.libPolkaRated($u);
    case 'friends':
      if (arg(2) == 'books')
        return $r.libPolkaFriendsBooks();
     return $r.libPolkaFriends();
    case 'show': 
     return libPolkaShow(arg(2));
    case 'add':
      if (($b = 1*arg(2)) && $u) {
        $b = Sel ("BookId FROM libbook WHERE BookId = $b");
        $txt = addslashes($_POST['text']);
        $f = $_POST['Flag'] == 'true' ? 'h' : '';
        if (Sel ("ID FROM libpolka WHERE BookId = $b AND Userid = $u")) 
          Update ('libpolka',"Text = '$txt', Flag='$f'", "BookId = $b AND Userid = $u");
        else 
          Insert ('libpolka', 'BookId, UserId, Text, Flag', "$b, $u, '$txt', '$f'");
        libsettag(polka);  
      }
    break;
    case 'del':
      if (($b = 1*arg(2)) && $u) {
        db_query("DELETE FROM libpolka WHERE UserId = $u AND BookId = $b");
        libsettag(polka);  
      }  
    break;
    case 'delrd':
      if (($b = 1*arg(2)) && $u) 
        db_query("DELETE FROM libreaded WHERE UserId = $u AND BookId = $b");
    return $r.libPolkaReaded($u);
  }

  $r .= "<h3>Отложенные книги и комментарии к ним</h3>\n";
  $sth = SELECT ("* FROM libpolka WHERE UserId = $u ORDER BY 1 DESC");
  while ($a1 = dbf($sth)) {
    $b = $a1->BookId;
    $t = stripslashes($a1->Text);
    $ch = $a1->Flag == 'h' ? 'checked' : '';
    $r .= "<br>".DoLibRow($b, 'rate');
    $r .= "<textarea class=polka cols=80 rows=7 id=$b onchange=polkasave($b) onBlur=polkasave($b)>$t</textarea><br>
        <a href=javascript:polkasave($b)>(записать)</a> 
        <input id=h$b type=checkbox $ch onchange=polkasave($b) onBlur=polkasave($b)> спрятать 
        <a href=/polka/del/$b>(удалить)</a><hr>\n";
  }  

  return $r;
}

function polkaul($u) {
  return bwlist_icons($u)."<b><a href=/polka/show/$u>".Sel("name FROM users WHERE uid = '$u'")."</a></b>";
}

function libpline($a1, $u='', $rss=0) {
  if (!bwlist_see($a1->UserId)) return '';
  Global $host;
  $book = Sel("Title FROM libbook WHERE BookId = $a1->BookId");
  $a = Sel("AvtorId FROM libavtor WHERE BookId = $a1->BookId");
  $an = Sel("LastName FROM libavtorname WHERE AvtorId = '$a'");
  $t = stripslashes($a1->Text);
  if ($rss) {
    $h = "http://$host";
    $u = $a1->UserId;
    $user = Sel("name FROM users WHERE uid = $u");
    return "$user про $an: $book\n$t\n";
  } elseif ($u) {
    $user = polkaul($a1->UserId)." про ";
  }  
  $t = preg_replace('/\n/', '<br>', $t);
  return "$user <a href=\"$h/a/$a\">$an</a>: <a href=\"$h/b/$a1->BookId\">$book</a><br>$t<hr>\n";
}  

function libPolkaShow($u) {
  Global $rates;
  Global $user;
  if ($u == 'all') {
    drupal_set_html_head('<link rel="alternate" type="application/rss+xml" title="Новые впечатления на Либрусеке RSS" href="/polka/show/all/rss" />'); 
    set_title("Впечатления о книгах");
    $r .= "<a href=/polka/show/all/rss class=feed-icon><img src=/misc/feed.png alt=RSS title=RSS width=16 height=16></a><br>"; 
    $S = "FROM libpolka WHERE Flag = '' AND Text <> ''";
    if ($user->uid) 
      $mode = db_result(db_query("SELECT bw FROM {bwlist} WHERE uid = %d AND uid2 = 0", $user->uid));
    if ($mode == 'w') {
      $S = "FROM libpolka JOIN {bwlist} ON (UserId=uid2) WHERE uid='$user->uid' AND bw='w' AND Flag = '' AND Text <> ''";
    } elseif ($mode == 'b') {
      $bl .= bwlist_makewhere('UserId');
      if ($bl) $S = "FROM libpolka WHERE $bl AND Flag = '' AND Text <> ''"; 
    } 
    $sth = pgq("SELECT * $S ORDER BY ID DESC", Sel ("COUNT(*) $S"));
    while ($a1 = dbf($sth)) $r .= libpline($a1, 1);
    return $r."<br>".theme('pager');
  }    

  if ($u) $u = Sel("uid FROM users WHERE uid = '$u' OR name = '$u'");
  if (!$u) return "Пользователь $u не найден.";

  set_title(Sel("name FROM users WHERE uid = '$u'")." - книжная полка");
  $sth = SELECT("* FROM libpolka WHERE UserId = $u AND Flag = '' AND Text <> '' ORDER BY ID DESC");
  while ($a1 = dbf($sth)) $r .= libpline($a1);
  $r .= "<br><h3>Оцененные книги</h3>\n<table>\n";
  $sth = SELECT ("BookId, Rate FROM librate WHERE UserId = $u ORDER BY Rate DESC");
  while ($a1 = dbf($sth)) $r .= "<tr><th>$a1->Rate (".$rates[$a1->Rate].") &nbsp; <td>".DoLibRow($a1->BookId, 'rate nobreak author')."\n";
  $r .= "</table>\n";  
  return $r;
}

function libPolkaFriends() {
  Global $user; 
  set_title("Читатели со схожими вкусами");
  if (!$u = $user->uid) return "Данный сервис имеет смысл только для зарегистрированных пользователей"; 
  $r = "<table><tr><th>Читатель<th>Похожих оценок";
  $sth = SELECT ("SUM(1) AS s, l2.UserId AS u FROM librate AS l1 JOIN librate AS l2 
         WHERE l1.UserId = $u AND l2.UserId <> $u AND l2.BookId = l1.BookId AND ABS(l1.rate-l2.rate) < 2 
         GROUP BY 2 ORDER BY 1 DESC LIMIT 100");
  while ($a1 = dbf($sth)) $r .= "\n<tr><td>".polkaul($a1->u)."<td>$a1->s";
  return $r."</table>\n";
 }
 
function libPolkaFriendsBooks() {
  Global $user; 
  set_title("Рекомендации друзей");
  if (!$u = $user->uid) return "Данный сервис имеет смысл только для зарегистрированных пользователей"; 
  $r = "<table><tr><th><th>Книга<th>Хороших оценок";
  $sth = SELECT ("SUM(1) AS s, l2.UserId AS u FROM librate AS l1 JOIN librate AS l2 
         WHERE l1.UserId = $u AND l2.UserId <> $u AND l2.BookId = l1.BookId AND ABS(l1.rate-l2.rate) < 2 
         GROUP BY 2 ORDER BY 1 DESC LIMIT 30"); // люди с похожими вкусами
  while ($a1 = dbf($sth)) {
    $f = $a1->u;
    $totrated = Sel("SUM(1) FROM librate AS l1 JOIN librate AS l2 
                WHERE l2.BookId = l1.BookId AND l1.UserId = $u AND l2.UserId = $f"); // всего оценено товарищем $f из оценненых $u
    if ($totrated && $a1->s/$totrated > 0.6) $FRIENDS[$f] = $a1->s/$totrated;//определяем степень похожести, меньше 60% - это уже не единомышленник ниразу;
  }
  if (!$FRIENDS) return "Друзья не обнаружены. Оцените больше книг.";
  arsort($FRIENDS); // сортируем по похожести.
  while (list($f, $v) = each($FRIENDS)) {
    $sth = SELECT ("BookId, Rate-3 as r FROM librate WHERE UserId = $f AND Rate > 3");
    while ($a1 = dbf($sth)) 
      if (!$skipbook[$b = $a1->BookId]) {
        $wst = libWST($b);
        if (Sel ("BookId FROM libreaded WHERE UserId = $u AND $wst") OR libGetRate($b) OR !Sel("BookId FROM libbook WHERE BookId = $b")) {
          $skipbook[$b] = 1; // это мы уже читали
        } else {
          $BOOKS[$b] += $a1->r*3*$v*$v;  
          $RECOM[$b] ++;
        }  
      }
  }
  
  arsort($BOOKS);
  for ($i = 0; $i < 100 && (list($b, $v) = each($BOOKS)); $i++) 
    $r .=  "<tr><td>".DoLibRow($b, 'rateonly')."<td><a href=/b/$b>".DoLibRow($b, 'author')."</a><td>".$RECOM[$b]."\n";
  return $r."</table>\n";
}
 
function libPolkaReaded($u) {
  $r .= "<br><br><h3>Прочитанные неоценённые книги</h3>\n<ol>\n";
  if ($oo = $_GET['order']) SetUserOpt('p',$oo);
  else $oo = GetUserOpt('p');
  switch ($oo) {
    case 'a':
      $join = 'JOIN libavtor USING(BookId) JOIN libavtorname USING(AvtorId)';
      $order = 'LastName, FirstName';
      $g = "GROUP BY 1";
      $c1 = 'checked';
    break;  
    case 'n':
      $join = 'JOIN libbook USING(BookId)';
      $order = 'Title';
      $c2 = 'checked';
    break;  
    default:
      $order = 'Time DESC';
      $c3 = 'checked';
    break;  
  }
  $r .= "Сортировать: <form action=/polka/readed>
  <input type=radio name=order value=a $c1 onchange='this.form.submit()'>По автору 
  <input type=radio name=order value=n $c2 onchange='this.form.submit()'>По названию 
  <input type=radio name=order value=t $c3 onchange='this.form.submit()'>По времени прочтения</form>";
  $sth = pgq("SELECT BookId FROM libreaded LEFT JOIN librate USING (UserId, BookId) $join 
              WHERE UserId = $u AND ISNULL(rate)  
              ORDER BY $order", Sel ("COUNT(*) FROM libreaded WHERE UserId = $u"));
  while ($b = db_result($sth)) {
    if (Sel ("rate FROM librate WHERE BookId = $b AND UserId = $u")) continue;
    if ($c1) {
      $a = Sel ("AvtorId FROM libavtor WHERE BookId = $b");
      if ($a != $oa) {
        $r .= "<h4>".avl($a)."</h4>";
        $oa = $a;
      }  
    }    
    $r .= "<li>".DoLibRow($b, 'rate nobreak')." <a href=/polka/delrd/$b>(не читал)</a>\n";
  } 
  $r .= "</ol>\n";  
  return $r.theme('pager');
}

function libPolkaRated($u) {
  $r .= "<br><br><h3>Оценки</h3>\n<ol>\n";
  if ($oo = $_GET['order']) SetUserOpt('p',$oo);
  else $oo = GetUserOpt('p');
  switch ($oo) {
    case 'a':
      $join = 'JOIN libavtor USING(BookId) JOIN libavtorname USING(AvtorId)';
      $order = 'LastName, FirstName';
      $g = "GROUP BY 1";
      $c1 = 'checked';
    break;  
    case 'n':
      $join = 'JOIN libbook USING(BookId)';
      $order = 'Title';
      $c2 = 'checked';
    break;  
    default:
      $order = 'ID DESC';
      $c3 = 'checked';
    break;  
  }
  $r .= "Сортировать: <form action=/polka/rated>
  <input type=radio name=order value=a $c1 onchange='this.form.submit()'>По автору 
  <input type=radio name=order value=n $c2 onchange='this.form.submit()'>По названию 
  <input type=radio name=order value=t $c3 onchange='this.form.submit()'>По времени прочтения</form>";
  $sth = pgq("SELECT BookId FROM librate $join WHERE UserId = $u $g ORDER BY $order", Sel ("COUNT(*) FROM librate WHERE UserId = $u"));
  while ($b = db_result($sth)) {
    if ($c1) {
      $a = Sel ("AvtorId FROM libavtor WHERE BookId = $b");
      if ($a != $oa) {
        $r .= "<h4>".avl($a)."</h4>";
        $oa = $a;
      }  
    }    
    $r .= "<li>".DoLibRow($b, 'rate nobreak')."\n";
  } 
  $r .= "</ol>\n";  
  return $r.theme('pager');
}

function libPolkaShowAllRSS($a=0) {
  Global $host;
  $CacheFile = "cache/p/$a.rss";
  if ($fh = libOpenCacheFile($CacheFile, 3600*$hours)) {
    $host = preg_replace('/^www./','',$_SERVER['HTTP_HOST']);
    if ($host == 'lib7.rus.ec' or $host == 'lib6.rus.ec' or $host == 'lib5.rus.ec'
    or $host == 'lib4.rus.ec' or $host == 'lib1.rus.ec' or $host == '89.248.165.5') 
       $host = 'lib.rus.ec';
    $link = "<link>http://$host/polka/show/all</link>";
    if ($a > 0) $link = "<link>http://$host/a/$a/v</link>";
    fwrite($fh, '<?xml version="1.0" encoding="utf-8"?>
<rss version="2.0">
<channel>
<title>Новые впечатления на Либрусеке</title>'." 
$link
<description>Последние 50 впечатлений о книгах на Либрусеке</description>
");
    if ($a > 0) 
      $sth = SELECT ("libpolka.BookId AS BookId, UserId, Text, CONCAT(DATE_FORMAT(Time, '%a, %e '), LEFT(DATE_FORMAT(Time, '%M'),3), DATE_FORMAT(Time, ' %Y %T')) AS T 
      FROM libpolka JOIN libavtor USING (BookId) WHERE libavtor.AvtorId = $a AND Flag = '' AND Text <> '' ORDER BY libpolka.ID DESC LIMIT 50");
    else 
      $sth = SELECT ("BookId, UserId, Text, CONCAT(DATE_FORMAT(Time, '%a, %e '), LEFT(DATE_FORMAT(Time, '%M'),3), DATE_FORMAT(Time, ' %Y %T')) AS T 
      FROM libpolka WHERE Flag = '' AND Text <> '' ORDER BY ID DESC LIMIT 50");

    while ($a1 = dbf($sth)) {
      $an = libpline($a1, 1, 1);
      $book = Sel("Title FROM libbook WHERE BookId = $a1->BookId");
      fwrite($fh, format_rss_item($book, "http://$host/b/$a1->BookId", $an, 
         array('guid' => "http://$host/b/$a1->BookId/$a1->UserId", 'pubDate' => "$a1->T GMT")));
    }
    fwrite($fh, "</channel>\n</rss>\n");      
    fclose($fh);
  }
  ob_clean();
  header('Content-Type: application/rss+xml; charset=utf-8'); 
  echo libReadCacheFile ($CacheFile);
  exit;
}

