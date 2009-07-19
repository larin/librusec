<?php 
include_once drupal_get_path('module', 'librusec') .'/librusec.inc';

function libNew() {
  foreach (array(1, 2, 3) as $v) 
    if ($arg = addslashes(strtolower(arg($v)))) 
      if ($arg == 'rss') 
        $rss = 1;
      elseif (Sel ("BookId FROM libbook WHERE Lang = '$arg' LIMIT 1"))
        $l = $arg;
      elseif (Sel ("BookId FROM libbook WHERE FileType = '$arg' LIMIT 1"))
        $t = $arg;
    
  if ($rss) { // отрабатываем new/rss
    if ($l) {
       $lng = " ($l)";
       $lsql = " Lang = '$l' AND ";  
    }   
    if ($t) {
      $lng .= " ($t)";
      $lsql .= " FileType = '$t' AND ";  
    }   
    return libRSS($l ? "new/$l.$t" : 'new', 1, $lng, "new/$l", '', $lsql);
  }  

// c rss покончено, отрабатываем новости
  if ($l) {
    $lsql = "AND Lang = '$l'";  
    $rssmod = "/$l";
    $lng .= ".$l";
  }  
  if ($t) {
    $lsql .= "AND FileType = '$t'";  
    $rssmod .= "/$t";
    $lng .= ".$t";
  }
  drupal_set_html_head('<link rel="alternate" type="application/rss+xml" title="Новые книги на Либрусеке RSS" href="/new/rss'.$rssmod.'" />');
  Global $sa, $rd, $user;
  if (!$user->uid) $asql = "AND Lang = 'ru'";
  $r = "<form action='/new'><a href='/new/rss$rssmod' class='feed-icon'><img src='/misc/feed.png' alt=RSS title='RSS' width='16' height='16'></a> &nbsp; <b><a href='http://syndicated.livejournal.com/librusec/' style='color:black'>ЖЖ</a></b> &nbsp; ";
  $sth = SELECT("DISTINCT Lang FROM libbook WHERE Lang <> '' AND NOT (Deleted&1) ".($t ? " AND FileType = '$t'" : ""));
  while ($v = db_result($sth)) $sellang .= "<option value='$v'".($v==$l?" selected":"").">$v";
  $r .= "Язык: <select onChange=\"document.location='/new/'+options[selectedIndex].value+'/$t'\";><option>$sellang</select> ";
  $sth = SELECT("DISTINCT FileType FROM libbook WHERE NOT (Deleted&1) ". ($l ? " AND Lang = '$l'" : ""));
  while ($v = db_result($sth)) $seltype .= "<option value='$v'".($v==$t?" selected":"").">$v";
  $r .= "Формат: <select onChange=\"document.location='/new/'+options[selectedIndex].value+'/$l'\";><option>$seltype</select> ";

  $r .= AnnChkbox().'</form>';
  $pg = $_GET['page'];
  $CacheFile = 'cache/new/'.($sa?'ann.':'').($rd?'rd.':'').($asql?'anonin.':'')."$pg$lng.html";
  if ($fh = libOpenCacheFile($CacheFile, 1200)) {
//    $nn = Sel("count(*) FROM libbook WHERE NOT (Deleted&1) $lsql $asql");
    $nn = 100000;
    fwrite($fh, downloadall('up', $nn, $t).'<br>'); 
    if ($user->uid && $rd) {
      $joinreaded = "LEFT JOIN libreaded ON(UserId =$user->uid AND libreaded.BookId = libbook.BookId)";
      $wherereaded = "AND libreaded.UserId IS NULL";
    }
    $fields = "Lang, N, FileType, FileSize, Deleted, Title, Title1";
    $sth = pgq("SELECT libbook.BookId AS BookId, $fields, libbook.Time AS Time FROM libbook $joinreaded WHERE NOT (Deleted&1) $wherereaded $lsql $asql ORDER BY BookId DESC", $nn);
    while ($a1 = dbf($sth)) { 
      if ($tm = ptm($a1->Time))
        fwrite($fh, "<br><h4>$tm</h4>\n");
      fwrite($fh, DoLibRow($a1, 'genre authors annotation checkbox new'));
    }  
    fwrite($fh, downloadall('dn')); 
    fwrite($fh, "</form><br>".theme('pager'));
    fclose($fh);
  }
  return $r.libReadCacheFile($CacheFile);
}

function libSeq() {
  $time = microtime(1);
  $req = preg_replace ('/page=(\d+)/','',$_SERVER['REQUEST_URI']);
  preg_match ('/[^\d]+(\d+)\=?$/',$req, $m);
  $sid = addslashes(arg(1));
  if (!$sid) $sid = $_GET['se'];
  $t1 = (microtime(1) - $time)*1000;
  if ($sid) {
    $se = Sel("SeqId FROM libseqname WHERE SeqId = '$sid'");
    if (!$se) $se = Sel("SeqId FROM libseqname WHERE SeqName = '$sid'");
  }
  $t2 = (microtime(1) - $time)*1000;
  if (!$se) {
    if ($_GET['order'] === 'a') {$order = 'SeqName';$c1 = 'checked';}
    else {$order='1 DESC';$c2 = 'checked';}
    $r = "<form action=/s>Сортировать
<input type=radio name=order value=a $c1 onClick=submit()>По алфавиту 
<input type=radio name=order value=b onClick=submit() $c2>По количеству книг</form>";
  $sth = pgq("SELECT sum(1) AS s, libseqname.SeqId as SeqId, SeqName FROM libbook JOIN libseq USING(BookId) JOIN libseqname USING(SeqId) WHERE libseqname.SeqId > 100 AND NOT (Deleted&1) GROUP BY 2,3 ORDER BY $order", Sel("max(SeqId)-400 FROM libseqname"));
    while ($a1 = dbf($sth)) $r .= "<br><a href=/s/$a1->SeqId>$a1->SeqName</a> ($a1->s)";
    return $r.theme('pager');
  } 

  if ($_GET['order'] === 'a') {$order = 'Title';$c2 = 'checked';}
  elseif ($_GET['order'] === 't') {$order = 'BookId DESC';$c3 = 'checked';}
  else {$order='SeqNumb';$c1 = 'checked';}
  if ($_GET['sa']) { $sa = 'checked'; Global $PP; $PP/=2;}

  $SeqName = Sel("SeqName FROM libseqname WHERE SeqId = $se");
  $n = Sel ("COUNT(*) FROM libbook JOIN libseq USING(BookId) WHERE SeqId=$se AND NOT (Deleted&1)");
  if (user_access('библиотекарь')) {
    if (arg(2) == 'rename' && ($newseqname = $_POST['newname'])) {
      if ($id = Sel("SeqId FROM libseqname WHERE SeqName = '%s'", $newseqname)) return "<a href=/s/$id>Сериал $newseqname уже существует.</a>";
      LogAction("UPDATE libseqname SET SeqName='".addslashes($newseqname)."' WHERE SeqId = $se", "Rename Sequence $SeqName($se) to $newseqname", 
                "UPDATE libseqname SET SeqName='".addslashes($SeqName)."' WHERE SeqId = $se");
      $SeqName = $newseqname;
    }
    $r .= "<form method=POST action=/s/$se/rename><input name=newname value='".check_plain($SeqName)."'><input type=submit value='Переименовать'></form>";
    if ($n == 0) { //сериал пуст, библиотекарь может его удалить
      if (arg(2) == 'delete') {
        LogAction("DELETE FROM libseqname WHERE SeqId=$se", "Delete Sequence $SeqName($se)", 
                  "INSERT INTO libseqname(SeqId, SeqName) VALUES($se, '".addslashes($SeqName)."')");
        return "Сериал $SeqName($se) удалён из библиотеки.";
      } else {
        $r .= "<br><a href=/s/$se/delete>Удалить сериал</a>";
      }
    }  
  }
  set_title($SeqName);
  $S = "BookId FROM libbook JOIN libseq USING(BookId) WHERE SeqId=$se AND NOT (Deleted&1)";
  $t3 = (microtime(1) - $time)*1000;

  if ($n > 1) $r .= "<form action=/s/$se>Сортировать
<input type=radio name=order value=o $c1 onClick=submit()>По порядку 
<input type=radio name=order value=a $c2 onClick=submit()>По алфавиту 
<input type=radio name=order value=t onClick=submit() $c3>По дате поступления 
<input type=checkbox name=sa onClick=submit() $sa>Аннотации</form>";
else $sa = 'on';

  Global $lastSecId;
  $lastSecId = $se;
  $r .= downloadall('up', $n);
  $r .= "<br>";
  $fields = "Lang, N, FileType, FileSize, Deleted, Title, Title1";
  $sth = pgq ("SELECT $S ORDER BY $order", $n); 
  while ($a1=dbf($sth)) $r .= DoLibRow($a1->BookId, 'checkbox sequence annotation seq');
  $r .= downloadall('dn', $n, 0);
  $r .= "</form>";
  $t4 = (microtime(1) - $time)*1000;
  return $r.theme('pager'); 
}

function libDownload() {
  if (!$user->uid || !Sel("uid FROM libdonations WHERE uid = $user->uid"))
   if (LA() > 2) return "Извините, сервер сейчас перегружен. Зайдите через полчасика.";

  $ge = preg_replace ("/'/", '', $_GET['genre']);
  $ty = preg_replace ("/'/", '', $_GET['type']);
  $av = preg_replace ("/'/", '', $_GET['avtor']);
  if (!$ge || !$ty || !$av) return "<p>Эта страничка предназначена для желающих выкачать всю библиотеку, или заметную её часть.
<p>Осмысленность такогого желания сомнительна, но многие его проявляют.
<p>Если вам действительно хочется выкачать сотню гиг, включая кучу дублей - пожалуйста.
<p>Можно <a href=/allbooks>большими кусками</a>
<p>Помимо этого, можно выкачать два десятка гигабайт <a href=http://book.lib.rus.ec>из коллекции booklib</a>

<hr>
<form action=/download>
<p><b>Жанры:</b><input name=genre size=44> сюда впишите коды желаемых жанров (<a href=/ge>классификатор</a>) через запятую, например thriller, det_maniac. Если хочется всю фантастику, можно обобщить sf%,  если  всю научную литературу - sci%. Напишите all чтобы взять все жанры.
<p><b>Форматы:</b><input name=type size=44> сюда впишите допустимые форматы через запятую, например fb2 (почти всё худло), pdf, djvu или all чтобы взять всё.
<p><b>Авторы:</b><input name=avtor size=44> сюда впишите номера авторов через запятую, например 20781, 1775, 6116 или all чтобы взять всё.
<p><input type=submit value='сгенерировать список'>
<p>";

  if ($ge !== 'all') 
    foreach (split (',',$ge) as $g) {
      if ($wg) $wg .= ' OR ';
        $wg .= "GenreCode ".(strstr($g,'%')?'LIKE':'=')." '$g'"; 
    }
  if ($ty !== 'all') 
    foreach (split (',',$ty) as $t) {
      if ($wt) $wt .= ' OR ';
      $wt .= "FileType = '$t'"; 
    }
  if ($av !== 'all') 
    foreach (split (',',$av) as $a) {
      if ($wa) $wa .= ' OR ';
      $wa .= "AvtorId = '$a'"; 
    }     

  if ($wg) {
    $w = " JOIN libgenre USING(BookId) JOIN libgenrelist USING(GenreId) WHERE ($wg) ";
  } 
  if ($wa) {
    if ($w) $w = " JOIN libavtor USING(BookId) $w AND $wa";
    else $w = " JOIN libavtor USING(BookId) $w WHERE $wa";
  }

  if ($wt) {
    if ($w) $w .= ' AND ';
    else $w = 'WHERE ';
    $w .= "($wt)";
  }
  if ($w) $w .= ' AND '; 
  else $w = ' WHERE ';
  $w = "FROM libbook LEFT JOIN libfilename USING(BookId) $w NOT (Deleted&1)"; 
  $cnt = Sel ("Count(*) $w");
  $r = "<p>Всего нашлось около $cnt книг";
  $sth1 = pager_query("SELECT libbook.BookId AS b, Title, FileSize, FileName, FileType $w GROUP BY 1,2,3,4,5 ORDER BY 1 DESC", 10000, 0, "SELECT $cnt FROM libgenrelist");

  while ($a1=dbf($sth1)) { 
    $l = $a1->FileType == 'fb2' ? "/b/$a1->b/download" : '/b.usr/'.$a1->FileName;
    $r .=  "<br><a href=$l>$a1->Title ($a1->FileSize)</a>\n";
  }
//  LogAction('', "Mass get '$ge', '$ty','$av' - $cnt","SELECT BookId, Title, FileSize $w");
  return $r.theme('pager');
}

function libTools() {
  Global $user;
  switch (arg(1)) {  
    case 'ajax':
      if (isset($_GET['b'])) $b = (integer)$_GET['b'];
      $u = $user->uid;
      if (!$b || !$u) exit;
      switch($_GET['op']) {
        case 'setrate':
          $r = 1*($_GET['r']);
          if ($r == 0) Delete('librate WHERE BookId = %d AND UserId = %d', $b, $u);
          else if ($r > 0 && $r < 6) db_query("INSERT INTO librate (BookId, UserId, Rate) VALUES($b, $u, $r) ON DUPLICATE KEY UPDATE Rate=$r");
          libcd("librate$u");
        exit;

        case 'setquality':
          $q = 1*($_GET['qq']);
          if ($q == 0) Delete('libquality WHERE BookId = %d AND uid = %d', $b, $u);
          else db_query("INSERT INTO libquality (q, BookId, uid) VALUES ($q, $b, $u) ON DUPLICATE KEY UPDATE q=$q");
          bookquality($b, 1); //reset cache
        exit;
      } 
    exit;  

    case 'select':
    $f = "Select".arg(2)."Submit";
    return $f($_POST['proc'], addslashes($_POST['skip']));
    case 'createavtor':
     $n1 = trim(strtr ($_GET['FirstName'],'"',"'"));
     $n2 = trim(strtr($_GET['MiddleName'],'"',"'"));
     $n3 = trim(strtr($_GET['LastName'],'"',"'"));
     $n4 = trim(strtr($_GET['NickName'],'"',"'"));
     $pl = $_GET['proclink'];
     if (!$n3) return "Не бывает авторов без фамилии";
     $aid = AddAvtorId($n1,$n2,$n3,$n4);
     if ($aid) {
       if (strstr($pl, '//')) $d = str_replace('//',"/$aid/",$pl);
       else $d = "$pl/$aid";
       lgo($d);
     } else return "<p>Не удалось создать автора $n1 $n2 $n3 $n4";
     return '';
    case 'joinbook':
      foreach ($_GET as $p=>$k) {
        if ($k != 'on') continue;
        list ($t, $v) = split ('-', $p);
        if ($t == 'a')
          if ($a1) lgo ("a/$a1/alias/$v");
          else $a1 = $v;
        if ($t == 'b') 
          if ($b1) lgo ("b/$b1/join/$v");
          else $b1 = $v;  
      } 
    return 'нечего объединять';  
    case 'creategenre':
      drupal_mail('creategenre', 'lib@rus.ec', 'creategenre', arg(2)." for book http://lib.rus.ec/b/".arg(3), $user->mail);
      return "Ваше предложение о добавлении жанра отправлено администрации библиотеки и будет рассмотрено в недельный срок. Благодарим за сотрудничество.";
    case 'xml':
      $sth = pager_query("SELECT BookId FROM libbook WHERE FileType = 'fb2' AND NOT (Deleted&1) ORDER BY 1", 1000);
      $r = "<ol>"; 
      while ($a1 = dbf($sth)) {
        $b = $a1->BookId;
        $xml = simplexml_load_file("b/$b.fb2");
        if (!$xml) {
          $r .= "<li>$b <pre>".print_r(libxml_get_last_error(),1)."</pre>";
          system ("cp b/$b.fb2 tofix/$b.fb2");
        }  
     }
     return $r."</ol>".theme('pager');
  }
}

function libInstall() {
 switch (arg(1)) {
  case 'listtables';
    print "libactions:libavtor:libavtoraliase:libavtorname:libbook:libcache:libcache1:libdonations:libgenre:libgenrelist:libjoinedbooks:liblog:libmass:libpolka:libquality:librate:libreaded:libseq:libseqname:libstat:libuseropt";
    exit;
    $sth = SELECT("SHOW TABLES LIKE 'lib%'");
    while ($t = dbf($sth)) {
      if ($r) $r .= ':';
      $r .= $t[0];
      $r .= print_r($t, 1);
    }  
    print "<pre>";
    print $r;
  exit;      
  case 'create':
    mysql_connect('localhost', 'drupal', '');
    $a1 = db_fetch_array(db_query("SHOW CREATE TABLE ".arg(2)));
    print($a1['Create Table']); 
    exit;
  case 'fill': 
    $t = addslashes(arg(2));
    if (strstr($t, 'lib'))
      passthru ("mysqldump -u drupal drupal $t | grep INSERT");   
    exit;
  case 'listfiles':
    chdir ('modules/librusec');
    print join(':', glob("*inc"));
    exit;  
  case 'file':  
    chdir ('modules/librusec');
    print file_get_contents(preg_replace('|/|','-',arg(2)));
  exit;  
 }
}

