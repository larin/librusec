<?php  

include_once drupal_get_path('module', 'librusec') .'/librusec.inc';

function librusec_search_form() {
  $form['ask'] = array('#type' => 'textfield', '#title' => 'Название книги или фамилия автора', '#size' => 20, '#maxlength' => 40);
  $form['submit'] = array('#type' => 'submit', '#value' => 'Искать');
  return $form;
}

function librusec_search_form_submit($form) {
 return libSearch($form['ask']);
}

function libSearch($qqq = '') { 
  if ($q=$_GET['avt']) drupal_goto('a/'.$q);
  if ($q=$_GET['book']) drupal_goto('b/'.$q);
  if ($q=$_GET['seq']) drupal_goto('s/'.$q);
  if ($q=$_GET['genre']) drupal_goto('g/'.$q);
  Global $ask;
  $ask = $qqq;
  if (!$ask) $ask = trim($_GET['ask']);
  if (!$ask) $ask = arg(1);
  $ask = addslashes($ask);
  $searchform = "<br><br><br><hr>
  <form action=/booksearch>Поиск автора по фамилии: <a href=/a>lib.rus.ec/a/</a><input name=avt><input type=submit value='искать'></form>
  <form action=/booksearch>Поиск книги по названию: <a href=/b>lib.rus.ec/b/</a><input name=book><input type=submit value='искать'></form>
  <form action=/booksearch>Поиск сериала: <a href=/s>lib.rus.ec/s/</a><input name=seq><input type=submit value='искать!'></form>
  <form action=/booksearch>Поиск жанра по коду: <a href=/g>lib.rus.ec/g/</a><input name=genge><input type=submit value='искать'></form>
  <form action=http://gen.lib.rus.ec/search>Поиск научной литературы: <a href=gen.lib.rus.ec>gen.lib.rus.ec</a></form>
  <form action=http://lib.rus.ec/booklib>Поиск по файл-эхе SU.BOOKS:<a href=book.lib.rus.ec>book.lib.rus.ec</a><input name=a><input type=submit value='искать'></form>
  <form action=/booksearch>Общий поиск: <input name=ask value='$asc'><input type=submit value='искать!'></form>";
  if (!$ask) return $searchform;
  $r = "<p>Ищем: $ask</p>\n";
  if (strlen($ask) < 3) return "Слишком короткий запрос. Удлините.".$searchform;

  $cid = md5("search$ask".$_GET[page]);
  if ($cache = libcg($cid, 50000)) {
    apc_inc('Search hit');
    if ($cache[goto]) return lgo($cache[goto]);
    else return $cache[r].$searchform."!"; 
  }
  $key = sem_get(15, 10);
  if (!sem_acquire($key)) return;
  if ($cache = libcg($cid, 50000)) {
    apc_inc('Search hit');
    sem_release($key);
    if ($cache[goto]) return lgo($cache[goto]);
    else return $cache[r].$searchform."!!"; 
  }

  $q = "%$ask%";
  $q = preg_replace ('/\s+/','%',$q);
  $q = preg_replace ('/%+/', '%',$q);

  // Отрабатываем ситуацию Фамилия Название
  list($q1,$q2) = explode(' ',$ask);
  list($q3,$q4) = explode(' ',iconv ("windows-1251", "UTF-8", $ask));
  if (strlen($q1) > 4 && strlen($q2) > 2) {
    $st = SELECT ("libbook.BookId FROM libbook JOIN libavtor USING(BookId) JOIN libavtorname USING (AvtorId) WHERE NOT Deleted&1 AND (LastName ='$q1' OR LastName = '$q3') AND (Title = '$q2' OR Title = '$q4')");
    for ($i1 = 0; $b = db_result($st); $i1++, $bb = $b) $r .= DolibRow($b);
    if ($i1 == 1) {
      $cache = array('goto' => "b/$bb");
      libcs($cid, $cache, 'b');
      apc_inc('Search stored');
      sem_release($key);
      return lgo("b/$bb");
    }  
  }

  $h3style = "<h3 style='margin-top: 10px; margin-bottom: 5px;'>";

  if (!$_GET[page]) { // Сериалы
    $dbh = SELECT ("* FROM libseqname WHERE SeqName LIKE '$q'");
    for ($tots = 0; $a1=dbf($dbh); $tots++) { 
      if (!$tots) $r.= "$h3style Найденные сериалы:</h3>";
      $r.= "<li><a href=/sequence/$a1->SeqId>$a1->SeqName</a>";
      $path = "s/$a1->SeqId"; 
    } 
  }

  //авторы
  $w = " FROM libavtorname WHERE ";    
  if ($q2) $w .= " LastName = '$q1' AND FirstName = '$q2' OR LastName = '$q2' AND FirstName = '$q1' OR ";
  $w .= " FirstName = '$ask' OR LastName = '$ask' OR NickName = '$ask'";
  if ($tota = Sel ("COUNT(AvtorId) $w")) {
    $r .= "$h3style Найденные писатели:</h3>";
    $sth = pgq("SELECT * $w ORDER BY LastName, FirstName", $tota);
    while ($a1 = dbf($sth)) {
      $a = $a1->AvtorId;
      if (Sel("GoodId FROM libavtoraliase WHERE BadId=$a")) 
        $alias=$a;
      else {
        $r .= avline($a1); 
      }
//      $path = "a/$a"; 
    }
  }
  if ($alias) $r.= avline(S(" * FROM libavtorname WHERE AvtorId = $alias"));

  //книги
  $w1 = "WHERE (Title like '$q' OR Title1 like '$q') AND NOT (Deleted&1)";
  $totb = Sel("COUNT(*) FROM libbook $w1");
  if ($totb) {
    $r.= "$h3style Найденные книги:</h3>";
    $w1 = "FROM libbook LEFT JOIN libavtor USING (BookId) LEFT JOIN libavtorname USING (AvtorId) $w1";
    $sth1 = pgq("SELECT DISTINCT libbook.BookId AS b, Title, Title1, libavtor.AvtorId AS a, CONCAT_WS(' ', FirstName, MiddleName, LastName) as aname $w1 ORDER BY Title", max($tota, $totb)); 
    while ($a1 = dbf($sth1)) {
      $name = $a1->Title;
      if ($a1->Title1) $name .= " [$a1->Title1]";
      $name = preg_replace("/$ask/i","<b>$ask</b>",$name);
      $r.=  "<li><a href=/b/$a1->b>$name</a>";
      $r.=  " - &nbsp; <a href=/a/$a1->a>$a1->aname</a>";
      $path = "b/$a1->b"; 
    }
  }
  $tot = $tota + $totb + $tots; // всё что найдено

  if ($tot == 1 && !$_GET[page] && $path) { //найден только один объект. редиректим сразу на него.
      $cache = array('goto' => $path);
      libcs($cid, $cache, 'b');
      apc_inc('Search stored');
      sem_release($key);
      return lgo($path);
  }
  if ($tot < 33 && strlen($ask) > 5) {
    // если в основной базе мало чего нашли, поищем в буклибе
    if (Sel("1 FROM booklib LIMIT 1")) {
      $l = "LIKE CONVERT(_utf8 '$ask%' USING cp1251)";
      $st = SELECT ("* FROM booklib WHERE Title $l OR A1N1 $l OR A1N2 $l OR A2N1 $l OR A2N2 $l OR Seq $l LIMIT ".BPP());
      for ($i5 = 0; $a1 = dbf($st); $i5++) {
        if (!$i5) $r .= "$h3style Найдено в <a href=http://lib.rus.ec/booklib>BookLib</a></h3>";
        $r .= printline($a1); 
      }
    }
    $tot += $i5;
    // если всё ещё не нашли, поищем в колхозе
    if ($tot < 40 && Sel("1 FROM bookwarrior.updated LIMIT 1")) {
      $st = SELECT ("* FROM bookwarrior.updated WHERE (`Author` LIKE '%{$ask}%' OR `Title` LIKE '%{$ask}%') LIMIT ".BPP());
      for ($i6 = 0; $row = dbf($st); $i6++) {
        if (!$i6) $r .= "$h3style Найдено в <a href=http://gen.lib.rus.ec/>библиотеке Genesis</a></h3>\n";
        $title = stripslashes($row->Title);
        $author = stripslashes($row->Author);
        $vol = stripslashes($row->VolumeInfo);
        $publisher = stripslashes($row->Publisher);
        $year = $row->Year;
        $pages = $row->Pages;
        $lang = stripslashes($row->Language);
        $ident = stripslashes($row->Identifier);
        $edition = stripslashes($row->Edition);
        $ext = stripslashes($row->Extension);
        $size = $row->Filesize;
        if ($size >= 1024*1024*1024){
          $size = round($size/1024/1024/1024);
          $size = $size.' GB';
        } elseif ($size >= 1024*1024) {
          $size = round($size/1024/1024);
          $size = $size.' MB';
        } elseif ($size >= 1024){
          $size = round($size/1024);
          $size = $size.' kB';
        } else {
          $size = $size.' B';
        }
        $volinf = $publisher;
        if ($year) { if ($volinf) $volinf .= ', '; $volinf .= $year; }
        if ($pages) { if ($volinf) $volinf .= ', '; $volinf .= $pages.' '.$str_pp_ru; }
        if ($ident) { if ($volinf) $volinf .= ', '; $volinf .= 'ISBN '.$ident; }
        $vol_ed = $vol;
        if ($lang == 'Russian') $ed = ' '.$str_edition_ru;
        else $ed = ' '.$str_edition_en;
        if ($vol_ed) {
          if ($edition) $vol_ed = $vol_ed.', '.$edition.$ed;
        } elseif ($edition) {
          $vol_ed = $edition.$ed;
        }
        $volume = '';
        if ($vol_ed) $volume = " <font face=Times color=green><i>[$vol_ed]</i></font>";
        $volstamp = '';
        if ($volinf) $volstamp = " <font face=Times color=green><i>($volinf)</i></font>";
        $r .= "<li><a href='http://gen.lib.rus.ec/get?md5=$row->MD5'>$title$volume$volstamp</a> ($author) $size $ext";
      }
    }
  }

  if ($_GET[page] + $tot + $i6 == 0) $r.= "<p>Ничего не найдено. Введите фамилию автора или название книги для поиска";
  else $r .= "<br><br>".theme('pager');
  $cache = array('r' => $r);
  libcs($cid, $cache, 'b');
  apc_inc('Search stored');
  sem_release($key);
  return $r.$searchform;
}

function printline($a1) {
  $r = "<a href='http://lib.rus.ec/booklib/$a1->Filename'>$a1->Filename ($a1->Size b)</a> - ";
  $r .= "$a1->Title - ";
  if ($a1->Seq) $r .= "$a1->Seq $a1->SeqN - ";
  $r .= "$a1->A1N1 $a1->A1N2 $a1->A2N1 $a1->A2N2 - <font size=-70%>$a1->G1 $a1->G2 $a1->G3</font><br>";
  return "<li>$r";
}

function m($s) {
  if ($s) return $s.' ';
  else return '';
}

function avline($a1) {
  Global $ask;
  $a = $a1->AvtorId;
  $cnt = Sel("count(*) FROM libavtor JOIN libbook USING (BookId) WHERE AvtorId = $a AND NOT (Deleted&1)"); 
  if (!$cnt) {
    $GoodId = Sel("GoodId FROM libavtoraliase WHERE BadId=$a");
    if ($GoodId) $cnt = Sel("count(*) FROM libavtor JOIN libbook USING (BookId) WHERE AvtorId = $GoodId AND NOT (1&Deleted)"); 
  }
  if (!$cnt) return '';
  $name = "$a1->FirstName $a1->MiddleName $a1->LastName";
  if ($a1->NickName) $name .= ' ('.$a1->NickName.')';
  $name = preg_replace("/$ask/i","<b>$ask</b>",$name);  
  $r = "<li><a href=/a/$a>$name</a> ($cnt) ";
  if ($GoodId) $r .= "(синоним для ".avl($GoodId).")";
  return $r;
}  
