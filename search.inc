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
  Global $ask;
  $ask = $qqq;
  if (!$ask) $ask = trim($_GET['ask']);
  if (!$ask) $ask = arg(1);
  $ask = addslashes($ask);
  $searchform = "<br><br><br><hr><form action=/booksearch><input name=ask value='$asc'><input type=submit value='искать!'</form>";
  if (!$ask) return $searchform;
  $r = "<p>Ищем: $ask</p>\n";
  if (strlen($ask) < 3) return "Слишком короткий запрос. Удлините.".$searchform;
  $q = "%$ask%";
  $q = preg_replace ('/\s+/','%',$q);
  $q = preg_replace ('/%+/', '%',$q);

  // Отрабатываем ситуацию Фамилия Название
  list($q1,$q2) = explode(' ',$ask);
  list($q3,$q4) = explode(' ',iconv ("windows-1251", "UTF-8", $ask));
  if (strlen($q1) > 4 && strlen($q2) > 2) {
    $st = SELECT ("libbook.BookId FROM libbook JOIN libavtor USING(BookId) JOIN libavtorname USING (AvtorId) WHERE NOT Deleted&1 AND (LastName ='$q1' OR LastName = '$q3') AND (Title = '$q2' OR Title = '$q4')");
    for ($i1 = 0; $b = db_result($st); $i1++) $r .= DolibRow($b);
    if ($i1 == 1) lgo("b/$b");
  }

  if (!$_GET[page]) { // Сериалы
    $dbh = SELECT ("* FROM libseqname WHERE SeqName LIKE '$q'");
    for ($i2 = 0; $a1=dbf($dbh); $i2++) { 
      if (!$i2) $r.= "<h3>Найденные сериалы:</h3>";
      $r.= "<br><a href=/sequence/$a1->SeqId>$a1->SeqName</a>";
    } 
  }

    //авторы
  $w = " FROM libavtorname WHERE ";    
  if ($q2) $w .= " LastName = '$q1' AND FirstName = '$q2' OR LastName = '$q2' AND FirstName = '$q1' OR ";
  $w .= " AvtorId = '$ask' OR FirstName = '$ask' OR LastName = '$ask' OR NickName = '$ask'";
  if ($tot = Sel ("COUNT(*) $w")) {
    $r .= "<h3>Найденные писатели:</h3>";
    $sth = pgq("SELECT * $w ORDER BY LastName, FirstName", $tot);
    while ($a1 = dbf($sth)) {
      $a = $a1->AvtorId;
      if (Sel("GoodId FROM libavtoraliase WHERE BadId=$a")) 
        $alias=$a;
      else {
        $r .= avline($a1); 
      }
    }
  }
  if ($alias) $r.= avline(S(" * FROM libavtorname WHERE AvtorId = $alias"));

  //книги
  if ($q2) $w1 = "WHERE Title like '$q' AND NOT (Deleted&1)";
  else  $w1 = "WHERE libbook.BookId = '$ask' OR Title like '$q' AND NOT (Deleted&1)";
  $tot1 = Sel("COUNT(*) FROM libbook $w1");
  if ($tot < $tot1) $tot = $tot1;

  if ($tot1) {
    $r.= "<h3>Найденные книги:</h3>";
    $w1 = "FROM libbook LEFT JOIN libavtor USING (BookId) LEFT JOIN libavtorname USING (AvtorId) $w1";
    $sth1 = pgq("SELECT libbook.BookId AS b, Title, libavtor.AvtorId AS a, CONCAT_WS(' ', FirstName, MiddleName, LastName) as aname $w1 ORDER BY Title", $tot); 
    while ($a1 = dbf($sth1)) {
      $name = preg_replace("/$ask/i","<b>$ask</b>",$a1->Title);
      if ($a1->b != $lb) $r.=  "<br>\n<a href=/b/$a1->b>$name</a>";
      $r.=  " - &nbsp; <a href=/a/$a1->a>$a1->aname</a>";
      $lb = $a1->b;      
    }   
  }
  if ($i2+$tot == 0) $r.= "<p>Ничего не найдено. Введите фамилию автора или название книги для поиска";

  if ($i2+$tot+$tot1 >= 33 || strlen($ask) < 5) return $r."<br><br>".theme('pager').$searchform; // найдено достаточно

  // если в основной базе мало чего нашли, поищем в буклибе
  if (Sel("1 FROM booklib")) {
    $l = "LIKE CONVERT(_utf8 '$ask%' USING cp1251)";
    $st = SELECT ("* FROM booklib WHERE Title $l OR A1N1 $l OR A1N2 $l OR A2N1 $l OR A2N2 $l OR Seq $l LIMIT ".BPP());
    for ($i5 = 0; $a1 = dbf($st); $i5++) {
      if (!$i5) $r .= "<hr><p>Найдено в <a href=http://lib.rus.ec/booklib>BookLib</a><br>";
      $r .= printline($a1); 
    }
  }
    
    // если всё ещё не нашли, поищем в колхозе
    if ($i5+$i2+$tot+$tot1 < 40 && Sel("1 FROM bookwarrior.updated")) {
      $st = SELECT ("* FROM bookwarrior.updated WHERE (`Author` LIKE '%{$ask}%' OR `Title` LIKE '%{$ask}%') LIMIT ".BPP());
      for ($i = 0; $row = dbf($st); $i++) {
      if (!$i) $r .= "<hr><p>Найдено в <a href=http://gen.lib.rus.ec/>библиотеке Genesis</a></p>\n";
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
      if ($year) {
        if ($volinf) $volinf .= ', ';
        $volinf .= $year;
      }
      if ($lang == 'Russian') $pp = ' '.$str_pp_ru;
      else $pp = ' '.$str_pp_en;
      if ($pages) {
        if ($volinf) $volinf .= ', ';
        $volinf .= $pages.$pp;
      }
      if ($ident) {
        if ($volinf) $volinf .= ', ';
        $volinf .= 'ISBN '.$ident;;
      }
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
      $r .= "<a href='http://gen.lib.rus.ec/get?md5=$row->MD5'>$title$volume$volstamp</a> ($author) $size $ext<br>";
    }
  }
  return $r."<br><br>".theme('pager').$searchform;
}

function printline($a1) {
  $r = "<a href='http://lib.rus.ec/booklib/$a1->Filename'>$a1->Filename ($a1->Size b)</a> - ";
  $r .= "$a1->Title - ";
  if ($a1->Seq) $r .= "$a1->Seq $a1->SeqN - ";
  $r .= "$a1->A1N1 $a1->A1N2 $a1->A2N1 $a1->A2N2 - <font size=-70%>$a1->G1 $a1->G2 $a1->G3</font><br>";
  return $r;
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
  $r = "<a href=/a/$a>$name</a> ($cnt) <br>\n";
  if ($GoodId) $r .= "(синоним для ".avl($GoodId).")";
  return $r;
}  
