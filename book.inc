<?php
include_once drupal_get_path('module', 'librusec').'/librusec.inc';
include_once drupal_get_path('module', 'librusec').'/list.inc';
include_once drupal_get_path('module', 'node').'/node.pages.inc';
include_once drupal_get_path('module', 'librusec').'/polka.inc';

function libSaveLog($b, $u) {
  if (!$b) return;
  if ($u) db_query("INSERT INTO libreaded (BookId, UserId) VALUES($b, $u) ON DUPLICATE KEY UPDATE BookId = BookId");
  else {
    $u = $_SERVER['HTTP_X_REAL_IP'];
    if (!$u) $u = $_SERVER['REMOTE_ADDR'];
    if ($u) db_query("INSERT INTO liblog (BookId, UserId) VALUES($b, '$u') ON DUPLICATE KEY UPDATE BookId = BookId");
  }
}

function libBook() {
  Global $user, $ShowEN, $ttt, $jj; 
  $ttt[$jj++] = microtime(1);
  $bo = addslashes(arg(1));
  if ($bo > 0 && $bo == 1*$bo) {
    $b = Sel ("BookId FROM libbook WHERE BookId = %d", $bo);
  } elseif ($bo) {
//    if (!Sel("BookId FROM libbook WHERE Title LIKE '$bo%' AND NOT (Deleted&1) LIMIT 1"))
//      $bo = iconv ("windows-1251", "UTF-8", $bo);
    $sth = SELECT("BookId FROM libbook WHERE Title LIKE '$bo%' AND NOT (Deleted&1)");
    for ($n = 0; $bb = db_result($sth); $n++) $r .= '<li>'.bl($b=$bb).' - '.avl(Sel("AvtorId FROM libavtor WHERE BookId = $bb LIMIT 1"));
  }
  if ($n > 1) {
    set_title("Книги по названию $bo");
    return "<ol>$r</ol>";
  }
  $r = '';
  if (!$b) return libList('b', addslashes(arg(1)));
  
  if (!$ShowEN && Sel("Lang FROM libbook WHERE BookId = $b") != 'ru') return 'Что-то нерусское. Только для избранных пользователей.';
  $book = S("* FROM libbook WHERE BookId = $b");
  if (Sel("block FROM libblocked WHERE BookId = $b")) $book->block = 1;
  set_title($tit = $book->Title." ($book->FileType)");
  
  $u = $user->uid;
  if (arg(2) == 'get') return libDownloadBook($b);
  if ($u) $complain = " <a href=/b/$b/complain>(пожаловаться&nbsp;на&nbsp;плохое&nbsp;качество&nbsp;книги)</a>";

  $aname = Sel("CONCAT_WS(' ', FirstName, LastName) FROM libavtor JOIN libavtorname USING(AvtorId) WHERE BookId = $b LIMIT 1");
  $topadvert = "
<!-- topadvert.ru -->
<script type=\"text/javascript\">
var topadvert_params_array;
if( topadvert_params_array == null ){
	topadvert_params_array = new Array();
}
var topadvert_params = {
 feed_id: 2247, format: 2, yml: 1, pattern_id: 67, author: \"$aname\", name: \"$book->Title\" 
};
var topadvert_pattern_params = {
	div_id: 'topadvert_' + Math.round(Math.random()*100000),
	ref_id: 1668
};
topadvert_params_array[topadvert_params_array.length] = [topadvert_params, topadvert_pattern_params];
document.write('<div id=\"' + topadvert_pattern_params['div_id'] + '\" style=\"display: none\"></div>');
</script>
<!-- /topadvert.ru -->
";
$topadvert1 = '<!-- topadvert.ru -->
<script type="text/javascript">
document.write(\'<script type="text/javascript" charset="windows-1251" src="ht\'+\'tp://feed.topadvert.ru/load.js?nocache=\'+Math.round(Math.random()*100000)+\'"><\'+\'/scr\'+\'ipt>\');
</script>
<!-- /topadvert.ru -->';

  switch(arg(2)) {
    case 'read':
      libSaveLog($b, $u);
      return DoLibRow($b,'authors noread')." $mailit $complain<br>$topadvert<br>".libRead($b)."<br><br>".DoLibRow($b,'rateonly').$topadvert1;
    case 'download': 
      return libDownloadBook($b, GetUserOpt('D'));
    case 'fb2':
    case 'txt':
    case 'html':
    case 'rtf':
    case 'pdf':
    case 'pdb':
    case 'epub':
      return libDownloadBook($b, arg(2));
  } 

  if ($f = arg(2)) {
    if (!user_access('исправлять книги') || IsNewUser()) return NetPrav;
    if (!libcanedit($b) && $f != 'forum') 
      return "Файл достаточно хорош. Нет смысла в его улучшении<br><br>".
             SelectQuality($b, $user->uid).($book->FileType == 'fb2' ? 
               "<br><b>FB2 document-info</b>:<br>".
               parse_fb2_document_info("b/$b.fb2"):'');
    if (function_exists($f = "libBook$f"))
      return $f($b, $book);
    return "Непонятный вызов $f($b)";  
  }    

  $r .= Undellink($b);
  $ttt[$jj++] = microtime(1);
  if (user_access('библиотекарь')) {
    if ($book->block) $block = " &nbsp; <a href=/b/$b/block/0>(разблокировать)</a>";
    else $block = " &nbsp; <a href=/b/$b/block/1>(заблокировать)</a>";
    $sth = SELECT("BadId FROM libjoinedbooks WHERE GoodId = $b");
    while ($a1=dbf($sth)) $r .= "(замена для ".bl($a1->BadId).")<br>";
  }
  $sth = SELECT ("AvtorId FROM libavtor where BookId = $b");
  while ($a1=dbf($sth)) $r .= avl($a1->AvtorId)." &nbsp; ";
  $sth = SELECT ("TranslatorId FROM libtranslator where BookId = $b");
  for ($kk = 0;$a1=dbf($sth); $kk++) {
    if (!$kk) $r .= '(перевод:';
    else $r .= ',';
    $r .= ' '.avl($a1->TranslatorId);
  }  
  if ($kk) $r .= ')';
  $wst = libWST($b);
  $n = $book->N;
  $ttt[$jj++] = microtime(1);
  if (LA() < 0.5) {
    $n = Sel ("SUM(N) FROM libstat WHERE $wst") + Sel("SUM(1) FROM liblog WHERE $wst");
    if ($n != $book->N && $n > 0) Update (libbook, "N=$n", "BookId=$b");
  }  
  $ttt[$jj++] = microtime(1);
  $r .= DoLibRow($b, 'sequencen nolink genre nobreak seltype notrans');
  $ttt[$jj++] = microtime(1);
  if ($book->block) $r .= " <img src=/img/zamok.gif border=0 alt='Заблокировано'>";
  if ($book->Year) $r .= " &nbsp; издание $book->Year г. ";
  $publishseqs = SELECT("* FROM libseq JOIN libseqname USING(SeqId) WHERE BookId = $b AND level > 100");
  $seqtxt = "";
  while ($seq = dbf($publishseqs)) {
  if (!$seqtxt)
    $r .= " &nbsp; издано в серии ";
  else
    $seqtxt .= ", ";
    $seqtxt .= "<a href=/s/$seq->SeqId>$seq->SeqName</a>" ;
  }
  $r .= $seqtxt;  if (libcanedit($b)) $r .= " &nbsp; <a href=/b/$b/edit>(исправить)</a> $block";
  $r .= "<br>";
  $img = '';
  if ($book->FileType == 'fb2') {
    if (!$a1 = S("* FROM libcache WHERE BookId = $b")) {
      libRead($b);
      $a1 = S("* FROM libcache WHERE BookId = $b");
    }
    $toc = preg_replace('/<a href=#([^>]+)>/s', "<a href='/b/$b/read#$1'>", $a1->TOC);
    if ($toc) $toc = "<table border=0><tr><td><h2>Оглавление</h2>\n<ul>$toc</ul></table>";
    $host = 'http://'.$_SERVER['HTTP_HOST'];
    $img = preg_replace('/^\//','',$a1->Cover);
    if (!$img) $img = $a1->Cover;
    if ($img) {
      if (!file_exists($img)) $img = "i/".($b%100)."/$b/$img";
      if (file_exists($img)) {
        $isize = getimagesize($img); 
        if ($isize[0] > 300) $ww = "width=300";
        $imgf = '[img]'."$host/$img".'[/img]';
        $imgb = "&lt;img $ww border=0 align=left style='padding: 9px;' src=\"$host/$img\"&gt;";
        $img = "<img align=left style='padding: 9px;' $ww src=\"/$img\">"; 
      } else $img = ''; 
    }  
  }
  $r .= $img."<a href=javascript:show('link1')>(ссылка для форума (BBCode))</a> <a href=javascript:show('link2')>(ссылка для блога (HTML))</a><br>
<div id=link1 style='position:absolute;left:-4000px'><br><br>".'[url='."$host/b/$b".']<br>[b]'.$tit.'[/b]<br>'.$imgf.'['."/url]<br><br></div>
<div id=link2 style='position:absolute;left:-4000px'><br><br>&lt;a href=$host/b/$b&gt;<br>&lt;b&gt;$tit&lt;/b&gt;<br>$imgb&lt;/a&gt;<br><br></div>";

  $r .= $topadvert;
  $r .= "<h2>Аннотация</h2>\n";
  $ann = ReadAnnotation($b, $nid);

  if (!$ann) {
    $link = "/node/add/bdesc/$b";
    if ($oann) $r .= $oann."\n<p>(<a href=$link>создать аннотацию и тэги</a>)";
    else $r .= " отсутствует (<a href=$link>создать аннотацию и тэги</a>)";
  } else {
    $node = node_load($nid, NULL, TRUE);
    $terms = taxonomy_link('taxonomy terms', $node);
    $r .= "$ann\n<p>".theme('links', $terms)."<a href=/node/$nid/edit> (исправить аннотацию и тэги) </a>";
  }  
  $forum = Sel ("nid FROM node_revisions WHERE title like 'B$b %'");
  if (!$forum) $forum = $oforum;
  if ($forum) $comments = Sel ("COUNT(*) FROM comments WHERE nid = $forum") + 1; 
  if ($forum) $r .= "<a href=/node/$forum>(обсужается на форуме - $comments сообщений)</a>"; 
  else $r .= " <a href=/b/$b/forum>(обсудить на форуме)</a>"; 
  $r .= $complain."<br><BR><br>";
// советы  
  if ($a1 = S("Advise,  NOW() - Time as tm FROM libcache1 WHERE BookId = $b")) {
    $incache = 1;
    if ($a1->tm < 500000 or LA() > 0.5) foreach (split (',' , $a1->Advise) as $bi) $advise .= AdRow($bi);
    else $incache = 2;  
  } 
  if ($incache != 1 && LA() < 0.5) {
    $sth = SELECT (
    "l2.BookId AS B, count(l2.BookId) as N 
     FROM libreaded as l1 join libreaded as l2 
     WHERE l1.BookId = $b AND l2.BookId <> $b AND l1.UserId = l2.UserId 
     GROUP BY l2.bookid order by N desc limit 17");
    while ($a1 = dbf($sth)) {
      if ($ad) $ad .= ',';
      $advise .= AdRow($a1->B); 
      $ad .= $a1->B;
    }
    if ($ad)
      if ($incache) Update ('libcache1', "Advise='$ad'","BookId=$b");
      else Insert ('libcache1', "Advise, BookId", "'$ad',$b");
  }
  $ttt[$jj++] = microtime(1);
  $r .= printallann($b).$toc.$advise."<br><Br><script src=http://odnaknopka.ru/ok2.js></script>";
  $r .= $topadvert1;
  $ttt[$jj++] = microtime(1);
  return $r;
}

function AdRow($b) {
  Global $RowCnt; 
  $r = '';
  if (++$RowCnt == 1) $r = "<h2>Читатели, читавшие эту книгу, также читали:</h2>\n";
  if (Sel("NOT(Deleted&1) FROM libbook WHERE BookId = $b")) $r .= DoLibRow($b, 'authors');
  return $r;
}

function printallann($b) {
  Global $user; $u = $user->uid;
  $wst = libWST($b);
  $sth = SELECT ("* FROM libpolka WHERE $wst AND UserId <> '$u' AND text <> '' AND Flag <> 'h' ORDER BY ID DESC");
  $r = '';
  while ($a1 = dbf($sth)) $r .= libpline($a1, 1);
  $r .= "<div id='newann' class=' withright clear-block'><table width=100%><tr><td width=100%><h2>Впечатления о книге: &nbsp; ".DoLibRow($b,'rateonly')."</h2>\n";
  $a1 = S("count(Rate) AS N, SUM(Rate)/SUM(1) AS S, MAX(Rate) AS m1, MIN(Rate) AS m2 FROM librate WHERE $wst");
  if ($i = $a1->N) $r .= "<p>оценки: $i".($i > 1 ? ($a1->m1 != $a1->m2 ? ", от $a1->m1 до $a1->m2" : '').", среднее ":': ')." $a1->S</p>";

  if ($u) {
    $ch = Sel ("Flag FROM libpolka WHERE UserId = $u AND BookId = $b") == 'h' ? 'checked' : '';
    $r .= "<form action=/polka/add/$b method=POST><textarea name=text cols=60 rows=7 id=$b onchange=polkasave($b)>".stripslashes(Sel("Text FROM libpolka WHERE UserId = $u AND BookId = $b"))."</textarea>
    <br><input type=submit value='записать'>
    <input id=h$b type=checkbox $ch onchange=polkasave($b) onBlur=polkasave($b)> спрятать (не показывать среди аннотаций)
    </form><form action=/polka/del/$b><input type=submit value='убрать с полки'></form>\n";
  }
  $r .= '</table></div>';
  return $r;
}    

function libDownloadBook($b, $dtp = 'fb2') {
  Global $user;
  if (!$dtp) $dtp = 'fb2';
  $a1 = S("* FROM libbook WHERE BookId = $b");
  libSaveLog($b, $u);
  if ($a1->FileType == 'fb2') {
    if ($dtp == 'pdf') drupal_goto("http://www.fb2pdf.com/convert.php?auto=yes&url=http://lib.rus.ec/b/$b/fb2");
    $zip = MakeZipFile($b, $dtp);
    if ($zip) return drupal_goto($zip);
    else return "Не получилось изготовить файл";
  }
  $f = Sel("FileName FROM libfilename WHERE BookId = $b");
  if (file_exists("b.usr/$f")) return drupal_goto("b.usr/$f");
  if (file_exists($f)) return drupal_goto($f);
  if ($server = variable_get('librusec_mans','')) return drupal_goto("$server$f"); 
  return "не найден $f";
}

function libBookEdit($b) {
  Global $user; $u = $user->uid;
  $r = Undellink($b);
  $a1 = S("* FROM libbook WHERE BookId = $b");
  if ($N = $_GET['SeqNumb']) {
    $N = (integer) $N;
    $SeqId = 1*arg(3);
    $SeqNumb = Sel("SeqNumb FROM libseq WHERE SeqId = $SeqId AND BookId = $b");
    if ($N != $SeqNumb)
      LogAction ("UPDATE libseq SET SeqNumb=$N WHERE BookId = $b AND SeqId = $SeqId", "Set SeqNumb to $N", 
                 "UPDATE libseq SET SeqNumb='SeqNumb' WHERE BookId = $b", $b);
  } 
  if (($L = addslashes($_GET['Lang'])) && ($a1->Lang != $L)) {
    LogAction ("UPDATE libbook SET Lang='$L' WHERE BookId = $b", "Set Lang to $L", "UPDATE libbook SET Lang='$a1->Lang' WHERE BookId = $b", $b);
   $a1->Lang = $L;
  }
  if (($Type = addslashes($_POST['FileType'])) && ($a1->FileType != $Type)) {
    LogAction ("UPDATE libbook SET FileType='$Type' WHERE BookId = $b", "Set FileType to $Type", "UPDATE libbook SET FileType='$a1->FileType' WHERE BookId = $b", $b);
    $a1->FileType = $Type;
  }
       
  $r .= SelectQuality($b, $u);  
  $r .= "<br>\n";
       
  $r .= "\n<br><b>Прочитать книгу:</b>".DoLibRow($b);
  $askt = urlencode($a1->Title);
  $tit = check_plain($a1->Title);
  $tit1 = check_plain($a1->Title1);
  $r .= "\n<form method=POST action=/b/$b/settitle>
  Название: <input name=tit value=\"$tit\"> доп. инф.:[<input name=tit1 value=\"$tit1\">] 
  <input type=submit value='Сохранить новое название' $ButtonStyle></form>
  <a href=\"/booksearch?m=j&ask=$askt\">Поискать дубли</a>";
  if (user_access('библиотекарь') && $a1->FileType != 'fb2') 
    $r .= " <form method=POST action=/b/$b/edit><input name=FileType value=$a1->FileType><input type=submit value='Сохранить тип'></form>";
  $r .= "\n<form action=/b/$b/addgenre method=post id=genreform><b>Жанры:</b>";
  $s = SELECT ("* FROM libgenre JOIN libgenrelist USING (GenreId) WHERE BookId = $b");
  while ($a2 = dbf($s)) {
    $g = $a2->GenreId;
    $r .= " $a2->GenreCode $a2->GenreDesc <a href=/b/$b/delgenre/$g>(удалить жанр)</a>\n";
  }
    $r .= "<input name=srcgenre><input type=submit value='добавить жанр' $ButtonStyle></form>";

    $r .= "<h4>Авторы:</h4>\n";
    $s = SELECT ("* FROM libavtor JOIN libavtorname USING (AvtorId) WHERE BookId = $b");
    for ($an=0;$a2 = dbf($s);$an++) 
      $r .= avl($a = $a2->AvtorId)." <a href=/b/$b/delauthor/$a>(удалить)</a> \n";
    if ($bl && $user->uid != 1) return $r;
    $r .= " <a href=/b/$b/addauthor>(добавить автора)</a>\n";
    if ($an == 1) $r .= " <a href=/b/$b/XchangeAuthor>(заменить автора)</a>\n";

    $r .= "<h4>Переводчики:</h4>";
    $s = SELECT ("* FROM  libtranslator JOIN libavtorname ON (AvtorId=TranslatorId) WHERE BookId = $b");
    for ($an=0;$a2 = dbf($s);$an++) {
      $r .= avl($a = $a2->AvtorId)." <a href=/b/$b/deltranslator/$a>(удалить)</a> \n";
    }
    if ($bl && $user->uid != 1) return $r;
    $r .= " <a href=/b/$b/addtranslator>(добавить переводчика)</a>\n";
    if ($an == 1) $r .= " <a href=/b/$b/Xchangetranslator>(заменить переводчика)</a>\n";

    $s = SELECT("* FROM libseq JOIN libseqname USING(SeqId) WHERE BookId = $b");
    while ($ss = dbf($s)) {       
      $r .= "<form action=/b/$b/edit/$ss->SeqId><b>Сериал:</b><input size=1 length=2 name=SeqNumb value=$ss->SeqNumb>$ss->SeqName 
            (<a href=/b/$b/delseq/$ss->SeqId>удалить</a>)</form>";
      $ssnn++;
    } 
    if (!$ssnn) {
      if (preg_match('/\((.+)\s*-\s*(\d+)\)/', $tit, $m)) {
        $serval = trim($m[1]); $sern = $m[2];
      }
      $r .= "<form action=/b/$b/addseq><input name=sern value=\"$sern\" size=1><input name=ser value=\"$serval\">
             <input $ButtonStyle type=submit value='Добавить сериал'></form>"; 
    }
    $r .= "<b>Язык:</b> <input size=2 id=lang$b onChange=setlang($b) value='$a1->Lang'>";
    $r .= "<br><b>Год издания:</b> <input size=3 id=year$b onChange=setyear($b) value='$a1->Year'>";
    $r .= "<br>Аннотация ";
    $ann = ReadAnnotation($b, $nid);
    $link = $nid ? "/node/$nid/edit" : "/node/add/bdesc/$b";
     if (!$ann) $r .= " отсутствует";
     else $r .= "$ann\n";
     if ($a1->FileType == 'fb2') 
       $r .= "<br><b>FB2 document-info</b>:<br>".parse_fb2_document_info("b/$b.fb2");
  
  $r .= "<br><a href=$link>Добавить/исправить аннотацию</a>";
  $r .= "<br>Возможные варианты:"; 
  $r .= "<br><a href=/b/$b/join>Объединить с другой книгой (отработка дублей)</a>";
  $r .= "<br><a href=/a/$a/addbook/>Заменить на другую версию </a>";
  $r .= "<br>".delbooklink($b, $a1->Title). " (маловероятно, что это понадобится. Наша цель - много книг.)\n";
//  $r .= "<pre>".print_r($a1,1)."</pre>";
  return $r;
}

function libBookDelete($b) {
  DeleteBook($b);    
  $a = Sel ("AvtorId FROM libavtor WHERE BookId = $b");
  return "Книга $b удалена.<br>Если это не то, что надо было сделать, <a href=/b/$b/undel>ткните сюда для восстановления</a>.
<br><a href=/a/$a/edit>Продолжить обработку</a> автора ".avl($a);
}

function libBookUnDel($b) {
  UnDeleteBook($b);    
  $a = Sel ("AvtorId FROM libavtor WHERE BookId = $b");
  return "Книга $b восстановлена.<br>\n<br><a href=/a/$a/edit>Продолжить обработку</a> автора ".avl($a);
}

function libBookDelAlias($b) {
  $b2 = 1*arg(3);
  if ($b && $b2) {
    LogAction("DELETE FROM libjoinedbooks WHERE BadId = $b AND GoodId = $b2", "Unjoin books $b $b2", 
              "INSERT INTO libjoinedbooks (GoodId, BadId) VALUES ($b2, $b)", $b2);    
  }
  return lgo("/b/$b");
}
function libBookJoin($b1) {
  $b2 = arg(3);
  if (!$b2) {
    return "Вы считаете, что ". bl($b1)." является не более, чем ошибочным синонимом для другой имеющейся в библиотеке книги.\n".
           "Давайте исправим ситуацию.<br><br>Поищем кандитатов на должность правильной книги.</br>\n".SelectBook("b/$b1/join", "AND BookId <> $b1", Sel("Title FROM libbook WHERE BookId = $b1"));
  } 
  if ($b1 == $b2) return;
  $r = "<h3>Объединение книг $b1 и $b2</h3>\n";
  $a1 = S("* FROM libbook WHERE BookId = $b1;");
  $a2 = S("* FROM libbook WHERE BookId = $b2;");
  if (($a1->Deleted&1) && ! ($a2->Deleted&1)) {
//    if ($b = Sel("GoodId FROM libjoinedbooks WHERE BadId = $b2")) return "Книга $b2 уже объединена с книгой $b";
    if ($b = Sel("GoodId FROM libjoinedbooks WHERE BadId = $b1")) return "Книга $b1 объединена с книгой $b";
    LogAction("INSERT INTO libjoinedbooks (GoodId, BadId) VALUES ($b2, $b1)", "Join books $b2 $b1", "DELETE FROM libjoinedbooks WHERE GoodId = $b2 AND BadId $b1", $b1);
    libwst($b1, 1);
    libwst($b2, 1);
    return "Объединение свершилось. Проследуйте на ".bl($b2);
  }  
  if ($a1->Deleted&1) return Undellink($b1); 
  if ($a2->Deleted&1) return Undellink($b2); 
    
  $r .= "<table border><tr><th>".
  "<th>Описание книги $b1 (<a href=/b/$b1/edit>исправить</a>)".
  "<th>Описание книги $b2 (<a href=/b/$b2/edit>исправить</a>)".
  "<tr><td>Название<td>".DoLibRow($b1, 'mini')."<td>".DoLibRow($b2, 'mini').
  "<tr><td>Размер файла<td>$a1->FileSize<td>$a2->FileSize".
  "<tr><td>Тип файла<td>$a1->FileType<td>$a2->FileType".
  "<tr><td>Появился в библиотеке<td>".ptm($a1->Time, 0)."<td>".ptm($a2->Time, 0).
  "<tr><td>Авторы<td>".bookav($b1)."<td>".bookav($b2);
  if ($a1->SeqId || $a2->SeqId) {
    $r .= "<tr><td>Сериал<td>";
    if ($a1->SeqId) $r .=  $a1->SeqNumb.'. '.Sel("SeqName FROM libseqname WHERE SeqId = $a1->SeqId");
    $r .=  "<td>";
    if ($a2->SeqId) $r .= $a2->SeqNumb.'. '.Sel("SeqName FROM libseqname WHERE SeqId = $a2->SeqId");
  }
  $r .= "<tr><td>Язык<td>$a1->Lang<td>$a2->Lang";
  if ($a1->Year || $a2->Year) $r .= "<tr><td>Год издания<td>$a1->Year<td>$a2->Year";
  $r .=  "<tr><td>Жанры<td>".bookgenres($b1)."<td>".bookgenres($b2);
  $r .= "<tr><td>Аннотация<td>".ReadAnnotation($b1)."<td>".ReadAnnotation($b2);
  if ($a1->FileType === 'fb2' || $a2->FileType === 'fb2') {
    $r .= "<tr><td>Информация о fb2<td>";
    if ($a1->FileType === 'fb2') $r .= parse_fb2_document_info("b/$b1.fb2");
    $r .= "<td>";
    if ($a2->FileType === 'fb2') $r .= parse_fb2_document_info("b/$b2.fb2");
  }  
  $r .= "<tr><td><td><a href=/b/$b1/dojoin/$b2>Оставить эту</a><td><a href=/b/$b2/dojoin/$b1>Оставить эту</a><tr><td><td colspan=2 align=center><a href=/>Оставить обе</a></table>";
  return $r; 
}

function libBookDoJoin($b1, $b2) {
  $b2 *= 1;
  if (arg(3)) $b2 = 1*arg(3);
  if (!Sel("BookId FROM libbook WHERE BookId = ".(int)$b1)*Sel ("BookId FROM libbook WHERE BookId = ".(int)$b2)) return 'error';
  if ($b = Sel("GoodId FROM libjoinedbooks WHERE BadId = $b2")) return "Книга $b2 уже объединена с книгой $b";
  if ($b = Sel("GoodId FROM libjoinedbooks WHERE BadId = $b1")) return "Книга $b1 объединена с книгой $b";
  LogAction("INSERT INTO libjoinedbooks (GoodId, BadId) VALUES ($b1, $b2)", "Join books $b1 $b2", "DELETE FROM libjoinedbooks WHERE GoodId = $b1 AND BadId = $b2", $b2);
  libwst($b1, 1);
  libwst($b2, 1);
  
  if (!Sel("SeqId FROM libseq WHERE BookId = $b1")) {
    $sth = SELECT("* FROM libseq WHERE BookId = $b2");
    while($ss = dbf($sth)) {
      LogAction("INSERT INTO libseq (BookId, SeqId, SeqNumb, Level) VALUES ($b1, $ss->SeqId, '$ss->SeqNumb', '$ss->Level')", 
                "Copy Sequense info from $b2 to $b1", 
                "DELETE FROM libseq WHERE BookId = $b1 AND SeqId = $ss->SeqId", $b1);
    }
    libcd('libseq'.$b);          
  }  
  DeleteBook($b2);
  return Undellink($b2);
}

function libBookDelAuthor($b) {
  if ($a = Sel ("AvtorId FROM libavtorname WHERE AvtorId = %d", arg(3))) {
    $an = Sel ("count(*) FROM libavtor JOIN libavtorname USING (AvtorId) WHERE BookId = $b");
    if ($an < 2) 
      return "<p>Невозможно удалить единственного автора из книги $b; Если это неправильный автор, добавьте правильного, а потом удалите этого\n";
    BookDeleteAvtor($b, $a);
    return lgo("/b/$b/edit");
  }
}

function libBookAddAuthor($b) {
  if ($a = Sel("AvtorId FROM libavtorname WHERE AvtorId = '".arg(3)."'")) {
    BookAddAvtor($b, $a);
    return lgo("b/$b/edit"); 
  } else {
    return "<p>Выберите автора для книги".SelectCreateAvtor("b/$b/addauthor");
  }
}

function libBookXchangeAuthor($b) {
  Global $ProcLink;
  $oldavt = Sel ("AvtorId FROM libavtor WHERE BookId = $b");
  if ($a = Sel ("AvtorId FROM libavtorname WHERE AvtorId = '".arg(3)."'")) {
    BookAddAvtor($b, $a);
    BookDeleteAvtor($b, $oldavt);
    return lgo("b/$b/edit"); 
  } else {
    return "<p>Выберите автора для книги".SelectCreateAvtor("b/$b/XchangeAuthor");
  }
}

function libBookDelTranslator($b) {
  if ($a = Sel ("AvtorId FROM libavtorname WHERE AvtorId = %d", arg(3))) {
    BookDeleteTranslator($b, $a);
    return lgo("/b/$b/edit");
  }
}

function libBookAddTranslator($b) {
  if ($a = Sel("AvtorId FROM libavtorname WHERE AvtorId = %d", arg(3))) {
    BookAddTranslator($b, $a);
    return lgo("b/$b/edit"); 
  } else {
    return "<p>Выберите переводчика для книги".SelectCreateTranslator("b/$b/addtranslator");
  }
}

function libBookXchangeTranslator($b) {
  Global $ProcLink;
  $oldavt = Sel ("TranslatorId FROM libtranslator WHERE BookId = %d", $b);
  if ($a = Sel ("AvtorId FROM libavtorname WHERE AvtorId = '%d'", arg(3))) {
    BookAddTranslator($b, $a);
    BookDeleteTranslator($b, $oldavt);
    return lgo("b/$b/edit"); 
  } else {
    return "<p>Выберите переводчика для книги".SelectCreateTranslator("b/$b/XchangeTranslator");
  }
}

function libBookAddSeq($b) {
  Global $ProcLink;
  if ($ser = trim(str_replace('"',"'",$_GET['ser']))) {
    $SeqId = Sel ("SeqId FROM libseqname WHERE SeqName = \"$ser\"");
    $SeqNumb = (integer)$_GET[sern];       
    if (!$SeqId) 
      return "Сериал '$ser' не найден. <a href='/b/$b/createseq/$SeqNumb/$ser'>(создать)</a>"; 
  } else {
     $SeqId = Sel ("SeqId FROM libseqname WHERE SeqId = '".arg(3)."'");
     $SeqNumb = (integer)arg(4);
  }  
  if ($SeqId >0 && $SeqNumb !== '') {
    BookAddSeq($b, $SeqId, $SeqNumb);
    return lgo ("b/$b/edit");
  } else {
     return "<p>Выбор сериала для книги ".bl($b)."<br><br>".SelectSequence("/b/$b/addseq");
  }
}

function libBookDelSeq($b, $s) {
  BookDeleteSeq($b, $s);
  return lgo ("b/$b/edit");
}

function libBookCreateSeq($b) {
  $SeqNumb = arg(3);
  $SeqId=AddSeqId(arg(4));
  if ($SeqId >0 && $SeqNumb !== '') 
    BookAddSeq($b, $SeqId, $SeqNumb);
  return lgo ("b/$b/edit");   
}

function libBookSetFileSize($b) {
  Global $user;
  print "$user->name";
  $fs = Sel ("FileSize FROM libbook WHERE BookId = $b");
  $fn = GetFileName($b);
  $fs1 = filesize($fn);
  print "$fn : $fs1";
  if ($fs1 && $fs != $fs1) Update("libbook", "FileSize=$fs1", "BookId = $b");
  else print 'errorrrrr';
  exit;
}

function libBookSetTitle($b) {
  $newtit = $_POST['tit'];
  $newtit1 = $_POST['tit1'];
  $book = new DB;
  if ($newtit && ($newtit != $book->Title($b) || $newtit1 != $book->Title1($b))) 
    BookSetTitle($b, $newtit, $newtit1);
  return lgo("b/$b/edit");
}

function libBookAddGenre($b) {
  if ($_POST['srcgenre']) 
    return SelectGenreSubmit("/b/$b/addgenre");
  if ($g = Sel ("GenreId FROM libgenrelist WHERE GenreId = '".arg(3)."'")) {
    if ($g < 1 or $b < 1) break;
    if  (Sel ("GenreId FROM libgenre WHERE BookId=$b AND GenreId=$g"))
       return "<p>Жанр $g уже добавлен к книжке $b";
    BookAddGenre($b, $g);
    return lgo("b/$b/edit"); 
  } else {
    return "<p>Жанр для книги".SelectGenre("/b/$b/addgenre");
  }
}

function libBookDelGenre($b){
  BookDelGenre($b, arg(3));
  return lgo("b/$b/edit"); 
}

function libBookComplain($b, $b1) {
  Global $user;
  if ($forum = Sel ("nid FROM node_revisions WHERE title like 'V$b %'")) {
    drupal_goto("node/$forum");
    exit;
  }   

  $sth = SELECT("BadId FROM libjoinedbooks WHERE GoodId = $b");
  while ($a2 = dbf($sth)) {
    if ($forum = Sel("nid FROM node_revisions WHERE title like 'V$a2->BadId %'")) {
      drupal_goto("node/$forum");
      exit;
    }   
  }
  set_title("Ошибки в книге: $b1->Title");

  if (node_access('create', 'forum')) {
    $r = drupal_get_form('forum_node_form', array(
      'uid' => $user->uid, 
      'name' => (isset($user->name) ? $user->name : ''), 
      'type' => 'forum', 
      'language' => 'ru',
      'title' => "V$b $b1->Title",
    ));
    $r = preg_replace('/option value="155"/','option value="155" selected',$r);
    $r = preg_replace('/><.textarea>/', ">В книге <a href=/b/$b>$b1->Title</a> много ошибок.\nНапример:\n\n</textarea>",$r);
    return $r;
  }
}

function libBookForum($b, $b1) {
  Global $user;
  set_title("Обсуждение книги: $b1->Title");

  if (node_access('create', 'forum')) {
    $r = drupal_get_form('forum_node_form', array(
      'uid' => $user->uid, 
      'name' => (isset($user->name) ? $user->name : ''), 
      'type' => 'forum', 
      'language' => 'ru',
      'title' => "B$b $b1->Title",
    ));
    $r = preg_replace('/option value="120"/','option value="120" selected',$r);
    $r = preg_replace('/><.textarea>/', "><a href=/b/$b>$b1->Title</a>\n\n</textarea>",$r);
    return $r;
  }
}

function libBookMail($b) {
  Global $user;
  if ($user->mail) 
    return file_get_contents("http://pirat.ec/cgi-bin/send.pl?$b-$user->mail");
}

function libBookRAW($b) {
  $a1 = S("* FROM libbook WHERE BookId = $b");
  if ($a1->FileType != 'fb2') return 'Увы, не потдерживается исправление не-fb2 книг. Сконвертируйте в fb2 и перезалейте';
  $raw = file_get_contents("b/$b.fb2");
  if (!$raw) return "Файл b/$b.fb2 не прочитался.";
  if (stripos (substr($raw, 0, 128), 'encoding="windows-1251"')) 
    $raw = iconv('windows-1251', 'utf8',$raw);
  $i1 = stripos($raw, '<FictionBook');  
  $i2 = stripos($raw, '<body>'); //начало основного текста
  $i3 = stripos($raw, '</body>');//конец основного текста
  $i4 = stripos($raw, '<binary', $h2);//начало картинок
  if (!$i2 || !$i3) return "Совсем странный FB2. Лучше с ним оффлайн покопаться".
  $head = substr($raw, $i1, $i2-$i1);
  $head = substr($raw, $i1, $i2-$i1);
  $body = substr($raw, $i2+6, $i3-7);

  $r .= "<hr>$i1, $i2, $i3, $i4<hr>";
  
  $r .= "<form method=POST action=/b/$b/raw>
<p>Заголовок FB2:</p>
<textarea cols=170 rows=15 name=head>$head</textarea>
<p>Основной текст:</p>
<br><textarea cols=170 rows=150 name=body>$body</textarea>
";
  if ($h3 - $h2 > 10 || !$h3 && strlen($raw) - $h2 > 10) 
    $r .= "<p>Сноски:</p><textarea cols=170 rows=25 name=notes>".substr($raw, $h2+7, $h3)."</textarea>\n";
  if ($h3) 
    $r .= "<br>А ещё тут картинки есть.";
  $r .= "<input type=submit></form>";
  return $r;
}

function libBookAnnotation($b, $b1) {
  print ReadAnnotation($b, $nid);
  exit;
}

function SelectQuality($b, $u) {  
  Global $user;
  $q1 = Sel ("q FROM libquality WHERE uid = $u AND BookId = $b");// наша оценка  
  $q2 = Sel ("AVG(q) FROM libquality JOIN users_roles USING(uid) WHERE BookId = $b AND rid = 3"); // мнение админов
  $q3 = Sel ("AVG(q) FROM libquality WHERE BookId = $b");// общее мнение  
  $q4 = $q2 ? $q2 : $q3;  
  $r .= "Качество файла: <b>$q4</b> ";
  if (user_access('библиотекарь') || $u && !$q2) { // можно высказать своё мнение
    $r .= "<select onchange=setquality($b) id=q$b>";
    for ($i=0; $i<=5;$i++) {
      $o = $i ? $i : '';
      $r .= "<option value=$i";
      if ($o == $q1) $r .= ' selected';
      $r .= ">$o</option>";
    }
    $r .= "</select>";
  }
  if ($q3 && user_access('библиотекарь')) 
    $r .= " <a href=/stat/q/$b>Все оценки файла</a>";
  if (user_access('библиотекарь')) 
    $r .= " <a href=/stat/edit/b/$b>История файла</a>";
    
  return $r;
}  

function libBookBlock($b, $book) {
  if (arg(3)) { 
    LogAction("INSERT INTO libblocked (BookId, block) VALUES ($b, 1)", "Block book $b", "DELETE FROM libblocked WHERE BookId = $b");
    return "Книга ".bl($b)." заблокирована";            
  } else {
    LogAction("DELETE FROM libblocked WHERE BookId = $b", "Unblock book $b", "INSERT INTO libblocked (BookId, block) VALUES ($b, 1)");
    return "Книга ".bl($b)." разблокирована";            
  }  
}

function libBookValidate($b) {
  include_once drupal_get_path('module', 'librusec').'/validator.inc';
  $v = validate("b/$b.fb2");
  return "Проверка файла $b.fb2: $v->s";
}