<?php
include_once drupal_get_path('module', 'librusec').'/librusec.inc';
include_once drupal_get_path('module', 'librusec').'/list.inc';
include_once drupal_get_path('module', 'librusec').'/book.inc';
include_once drupal_get_path('module', 'librusec').'/polka.inc';
include_once drupal_get_path('module', 'node').'/node.pages.inc';

function libAvtor() {
  Global $user;
  $ua = $_SERVER['HTTP_USER_AGENT'];
  if (strstr($ua, 'Download') or strstr($ua, 'Wget')) {sleep (7); exit;}
  $av = arg(1);
  if (!$av) return libList('a');
  if ($av == 'all') return AllAvtors();
  $a = Sel("AvtorId FROM libavtorname WHERE AvtorId = '%d'", arg(1));
  if ($a) { // нам дали id. Попробуем сделать фамилию
    $av = Sel("LastName FROM libavtorname WHERE AvtorId = '%d'", $a);
    $sth = SELECT("AvtorId FROM libavtorname WHERE LastName = '%s'", $av);
    for ($n = 0; $n < 3 && ($aa = db_result($sth));) 
      if (!Sel("GoodId FROM libavtoraliase WHERE BadId = %d", $aa)) 
        if ($cnt = Sel ("COUNT(*) FROM libavtor JOIN libbook USING(BookId) WHERE AvtorId = %d AND NOT (Deleted&1)", $aa))
          $n++;
    if ($n != 1 || strpos($av, ' ')) $av = $a;
  } else { // нам дали фамилию. Попробуем сделать id
    $a = Sel("AvtorId FROM libavtorname WHERE LastName = '%s' OR '%s' = Concat_WS(' ', FirstName, LastName) LIMIT 1", $av, $av);
    if (!$a) $av = iconv ("windows-1251", "UTF-8", $av);
    $sth = SELECT("AvtorId FROM libavtorname WHERE LastName = '%s' OR '%s' = Concat_WS(' ', FirstName, LastName)", $av, $av);
    for ($n = 0; $n < 10 && ($aa = db_result($sth));$k++) 
      if (!Sel("GoodId FROM libavtoraliase WHERE BadId = $aa")) 
         if ($cnt = Sel ("COUNT(*) FROM libavtor JOIN libbook USING(BookId) WHERE AvtorId = $aa AND NOT (Deleted&1)")) {
           $n++; $r .= '<li>'.avl($a=$aa)." ($cnt)";
         }
    if (!$a) return libList('a', $av);
    if ($n > 1) {
      set_title("Авторы по фамилии $av");
      return "<ol>$r</ol>";
    }
  }  
  if (arg(2) == 'rss') return libAvtorRSS($a);
  set_title($aname=AvtorName($a));
  $a1 = S("* FROM libavtorname WHERE AvtorId = $a");
  $a1->av = $av;
  $a1->aname = $aname;
  $f = arg(2);
  if ($f && $f != 'stat') 
    if (libcanedit()) 
      if (function_exists($f = "libAvtor$f"))
        return $f($a, $a1);

  $r = '';  
  $GoodId = Sel ("GoodId FROM libavtoraliase WHERE BadId = $a");
  if ($GoodId && $GoodId != $a) $r .= '<h2>(синоним для '.avl($a = $GoodId).')</h2>';
  if (strlen($lang=arg(2)) == 2) {
    $lsql = " AND Lang = '$lang'";
    $r .= "<h2>Язык: $lang</h2><br>";
  } else {
    $lang = '';
  } 
  drupal_set_html_head('<link rel="alternate" type="application/rss+xml" title="'.$aname.': новые книги на Либрусеке RSS '.$lang.'" href="'."/a/$a/rss/$lang".'" />');
  $r .= "<a href=/a/$a/rss/$lang class=feed-icon><img src=/misc/feed.png alt=RSS title=RSS width=16 height=16></a> <script src=http://odnaknopka.ru/ok2.js></script> "; 
  if ($uid=$a1->uid) {
    $uname = Sel ("name FROM users WHERE uid = $uid");
    $r .= "Известен в библиотеке как <a href=/user/$uid>$uname</a><br>";
  }  
  if (!$_GET['page'] && !$lang) {
    $ann = ReadAvtorPage($a, $nid);
    if (!$nid) $r .= "<a href=/node/add/adesc/$a>(добавить описание автора)</a>\n";
    else $r .= "$ann\n<p><a href=/node/$nid/edit>(исправить описание автора)</a>\n";
  }  
  if (libcanedit()) $r .= " <a href=/a/$a/edit>(исправить библиографию)</a> <a href=/a/$a/addbook/>(добавить книгу)</a>";
  $forum = Sel ("nid FROM node_revisions WHERE title like 'A$a %'");
  if ($forum) $comments = Sel ("COUNT(*) FROM comments WHERE nid = $forum") + 1; 
  if ($forum) $r .=  "<a href=/node/$forum>(обсужается на форуме - $comments сообщений)</a>"; 
  else $r .=  " <a href=/a/$av/forum>(обсудить на форуме)</a>"; 
  if (!$a1->NoDonate && ($a1->uid || $a1->WebPay)) $r .=  " <a href=/donate/author/$a>(заплатить автору)</a>";
//  $r .= " <a href=/a/$av/stat>(статистика)</a>";
  $r .= '<br>';

  if ($oo = $_GET['order']) SetUserOpt('o',$oo);
  else $oo = GetUserOpt('o');
  if ($oo == 'a') {
    $order = 'Title';
    $c1 = 'checked';
  } elseif ($oo == 't') {
    $order = '1 DESC';
    $c3 = 'checked';
  } else {
    $c2 = 'checked'; 
    $joinseq = "LEFT JOIN libseq USING(BookId) LEFT JOIN libseqname USING(SeqId)";
    $order = "SeqName, SeqNumb, 2, Title";
  }

  if (arg(2) == 'stat') return libAvtorStat($a, $c1, $c2, $c3, $order);

  if ($genre = $_GET['Genre']) {
    $g = Sel ("GenreId FROM libgenrelist WHERE GenreCode = '$genre'");
    $gn = Sel ("GenreDesc FROM libgenrelist WHERE GenreCode = '$genre'");
    $n = Sel ("Count(*) FROM libavtor JOIN libbook USING (BookId) JOIN libgenre USING (BookId) WHERE AvtorId = $a AND NOT (Deleted&1) AND GenreId = $g") + 
         Sel ("Count(*) FROM libtranslator JOIN libbook USING (BookId) JOIN libgenre USING (BookId) WHERE TranslatorId = $a AND NOT (Deleted&1) AND GenreId = $g");
  } else {
    $n = Sel ("Count(*) FROM libavtor JOIN libbook USING (BookId) WHERE AvtorId = $a AND NOT (Deleted&1)") + 
         Sel ("Count(*) FROM libtranslator JOIN libbook USING (BookId) WHERE TranslatorId = $a AND NOT (Deleted&1)") ;
  }

  if (!$lang) {
    $sth1 = SELECT("DISTINCT Lang FROM `libavtor` JOIN libbook USING (BookId) WHERE AvtorId = $a AND Lang <> ''");
    while ($l = db_result($sth1)) { $sellang .= "<option value='$l'>$l";  $ln++; }
  }  
  if ($ln > 1) $r .= "<br>Язык: <select onChange=\"document.location='/a/$a/'+options[selectedIndex].value\";><option>$sellang</select>";
if ($n > 1) $r .= '
<form action='.$formlink.'>Сортировать
<input type=radio name=order value=a '.$c1.' onClick="submit()">По алфавиту 
<input type=radio name=order value=b onClick="submit()" '.$c2.'>По сериям 
<input type=radio name=order value=t onClick="submit()" '.$c3.'>По дате поступления<br>'.AnnChkbox().'</form>';
else $sa = 'on';

  Global $sa, $rd;
  if ($user->uid && $rd) {
    $joinreaded = "LEFT JOIN libreaded ON(UserId =$user->uid AND libreaded.BookId = libbook.BookId)";
    $wherereaded = "AND libreaded.UserId IS NULL";
  }
  if ($joinseq) $seqid = ', SeqId';
  if ($g) {
    $sth = SELECT ("libbook.BookId AS b, GenreId $seqid
FROM libavtor JOIN libgenre USING (BookId) JOIN libbook USING (BookId) $joinseq $joinreaded
WHERE AvtorId = $a AND NOT (Deleted&1) $wherereaded AND GenreId = $g $lsql
GROUP BY 1 ORDER BY $order");
    $r .= "<h3>Произведения в жанре <a href=/g/$genre>$gn</a></h3>";
    $r .= "(<a href=/a/$av>все книги автора</a>)"; 
    $formlink = "/a/$av?Genre=$genre";
    $sth1 = SELECT ("libtranslator.BookId AS b, GenreId $seqid
FROM libtranslator JOIN libgenre USING (BookId) JOIN libbook USING (BookId) $joinseq $joinreaded
WHERE TranslatorId = $a AND NOT (Deleted&1) $wherereaded AND GenreId = $g $lsql
GROUP BY 1 ORDER BY $order");
  } else {
    $sth = SELECT("libbook.BookId AS b, GROUP_CONCAT(CAST(GenreId AS CHAR)ORDER BY GenreId) as g $seqid
FROM libavtor LEFT JOIN libgenre USING (BookId) JOIN libbook USING (BookId) $joinseq $joinreaded
WHERE AvtorId = $a AND NOT (Deleted&1) $wherereaded $lsql
GROUP BY 1 ORDER BY $order");
    $sth1 = SELECT("libtranslator.BookId AS b, GROUP_CONCAT(CAST(GenreId AS CHAR)ORDER BY GenreId) as g $seqid
FROM libtranslator LEFT JOIN libgenre USING (BookId) JOIN libbook USING (BookId) $joinseq $joinreaded
WHERE TranslatorId = $a AND NOT (Deleted&1) $wherereaded $lsql
GROUP BY 1 ORDER BY $order");
  $formlink = "/a/$av";
}

  $r .= downloadall('up', $n); 
  while ($a1 = dbf($sth)) 
    $r .= DoLibRow($a1->b, 'genre sequence annotation rate'. ($n > 1?'checkbox':''));
  for ($kk=0; $a1 = dbf($sth1);$kk++) {
    if (!$kk) $r .= "\n<h3>Переводы</h3>";
    $r .= DoLibRow($a1->b, 'genre sequence annotation rate'. ($n > 1?'checkbox':''));
  }
    
  $r .= downloadall('dn', $n)."</form>";//. theme('pager');

  $sth = SELECT ("* FROM libpolka JOIN libavtor USING (BookId) WHERE libavtor.AvtorId = $a AND Flag = '' AND Text <> '' ORDER BY libpolka.ID DESC LIMIT 10");
  while ($a1 = dbf($sth)) {
    if (!$ii++) $r .= "<br><br><h3><a href=/a/$a/v/rss class=feed-icon><img src=/misc/feed.png alt=RSS title=RSS width=16 height=16></a>
<a href=/a/$a/v>Впечатления</a></h3>";
    $r .= libpline($a1, 1);
  }  
  return $r;
}

function libAvtorV($a) {
  if (arg(3) == 'rss') return libPolkaShowAllRSS($a);
  $r .= "<h3><a href=/a/$a/v/rss class=feed-icon><img src=/misc/feed.png alt=RSS title=RSS width=16 height=16></a> Впечатления о книгах</h3>";
  $sth = SELECT ("* FROM libpolka JOIN libavtor USING (BookId) WHERE libavtor.AvtorId = $a AND Flag = '' AND Text <> '' ORDER BY libpolka.ID DESC");
  while ($a1 = dbf($sth)) {
    $r .= libpline($a1, 1);
  }  
  return $r;
}

function libAvtorEdit($a, $a1) {
  if (!libcanedit()) return NetPrav;
  Global $user, $sa, $ButtonStyle;
  $tot = Sel ("Count(*) FROM libavtor JOIN libbook USING (BookId) WHERE AvtorId = $a AND NOT (Deleted&1) ");
  $i = $tot==1?'и':'';
  $aname = "$a1->FirstName $a1->MiddleName $a1->LastName";
  if ($a1->NickName) $aname .= " ($a1->NickName)";
  $r .= "<h4><a href=/a/$a>$aname</a>, автор $tot книг$i</h4>";
  $r .= "<a href=/a/$a/correct>(переименовать)</a> &nbsp; <a href=/a/$a/alias>(объединить с другим автором)</a> ";

  if ($uid=$a1->uid) $uname = Sel ("name FROM users WHERE uid = $uid");
  $r .= "<form>Известен в библиотеке как <input size=40 id=uid$a onChange=setuid($a) value='$uname'></form>";

  if (Sel("libbook.BookId FROM libavtor JOIN libbook USING(BookID) WHERE AvtorId = $a AND (Deleted&1) LIMIT 1"))
    $r .= " <a href=/a/$a/deleted>(показать удалённые книги)</a> ";
  if (Sel("Sum(1), SUBSTRING_INDEX(Title, '(',1) as T FROM libavtor JOIN libbook USING(BookID) WHERE AvtorId = $a  AND NOT (Deleted&1) GROUP BY T ORDER BY 1 DESC LIMIT 1")>1) 
  $r .= "<a href=/a/$a/searchd>Показать преположительные дубли</a> ";
 
  $sth = SELECT("* FROM `libavtoraliase` WHERE GoodId = $a");
  while ($a1 = dbf($sth)) {
    $r .= "<br>".avl($a1->BadId)." => $aname ";
    if (user_access('библиотекарь')) $r .= "<a href=/a/$a/deletealias/$a1->BadId>(удалить алиас)</a>";
  }
 
  if ($oo = $_GET['order']) SetUserOpt('o',$oo);
  else $oo = GetUserOpt('o');

  if ($oo == 'a') {
    $order = 'Title';
    $c1 = 'checked';
  } elseif ($oo == 't') {
    $order = '1 DESC';
    $c3 = 'checked';
  } else {
    $order = "SeqId DESC, SeqNumb, 2, Title";
    $c2 = 'checked';
  }  
  $r .= "<form action=/a/$a/edit>Сортировать
<input type=radio name=order value=a '.$c1.' onClick=submit()>По алфавиту 
<input type=radio name=order value=b onClick=submit()$c2>По сериям и жанрам
<input type=radio name=order value=t onClick=submit()$c3>По дате поступления".AnnChkbox()."</form>
<table><form name=bk action=/mass/edit/$a>";
  $s1 = pgq("SELECT libbook.BookId AS b, GROUP_CONCAT(CAST(GenreId AS CHAR) ORDER BY GenreId) AS G, Lang  
             FROM libavtor JOIN libgenre USING (BookId) JOIN libbook USING (BookId) LEFT JOIN libseq USING(BookId) 
             WHERE AvtorId = $a AND NOT (Deleted&1) GROUP BY 1 ORDER BY $order", $tot);

  while ($a1 = dbf($s1)) {
    if (!$iiii++) {
      $r .= "<tr><td align=right colspan=3><input type=submit name=changegenre $ButtonStyle value='Изменить жанр у всех отмеченных книг на '> &nbsp; <select name=genre><option value=0>";
      $s2 = SELECT("GenreCode, GenreDesc FROM libgenrelist ORDER BY GenreMeta");
      while ($a3=dbf($s2)) $r .= "<option value=$a3->GenreCode>$a3->GenreDesc</a>";
      $r .= '</select><td><input type=checkbox name=all onClick="{UCC(this.checked)}">';
    } 
    $b = $a1->b;
    $r .= DoLibRow ($b,"genre edit nobreak");
    $r .= "<input size=2 id=lang$b onChange=setlang($b) value='$a1->Lang'> ";
    $r .= "<input type=checkbox name=$b></td></tr>\n";
    if ($sa) if ($an = ReadAnnotation($b, $nid)) $r .= "<tr><td colspan=3>$an";
  } 
  $r .= "<tr><td align=right colspan=3>
<input type=submit name=setseq $ButtonStyle value='Прописать сериал'>
<input type=submit name=deleteallnah $ButtonStyle value='Поудалять все отмеченное' onClick='return confirmmassdelete()'> 
<input type=submit name=join $ButtonStyle value='Сравнить пару книг'>
<td><input type=checkbox name=all onClick='{UCC(this.checked)}'></form></table>";
  return $r.theme('pager');
}

function libAvtorDeleted($a) {
  Global $user;
  if (!libcanedit()) return NetPrav;
  $st = SELECT("libbook.BookId AS b FROM libavtor JOIN libbook USING (BookId) WHERE AvtorId = $a AND (Deleted&1) GROUP BY 1");
  $r .= "<form name=bk action=/mass/undel/$a>";
  while ($a1 = dbf($st)) $r .= DoLibRow ($a1->b, 'checkbox');
  $r .= "<input type=checkbox name=all onClick='{UCC(this.checked)}'> &nbsp; <input type=submit name=restoreall $ButtonStyle value='Восстановить все отмеченное' onClick='return confirmmassundelete()'> 
</form>";
  return $r;
}

function libUpload() {
  Global $user;
  if (!libcanedit()) return NetPrav;
  if ($_POST['filetype'] || $_POST['ftpfile']) return libAvtorAddBook(0);
  switch (arg(1)) {
    default: return "
<ol>Правила
<li>Перед добавлением книги убедись, что этой книги в библиотеке нет или предлагаемая версия лучше.
<li>Если заменяешь имеющуюся версию fb2 на исправленную, увеличь номер версии.
<li>Если вы не в состоянии из имеющегося у вас текста создать нормальный ФБ2-файл, а текст нужный, то заливайте прямо в том виде, который попал вам в руки (txt, rtf и т.д) и предоставьте вопрос конвертации тем, кто это умеет и может делать качественно.
<li>Если вы взяли текст с СИ - укажите, что текст взят с СИ, и добавьте к описанию автора ссылку на его страничку в СИ (если этой ссылки или вообще описания ещё нет). Не заливайте сюда незаконченные и неполные произведения!
<li>FB2 должен иметь <a href=/g>жанр из списка</a>
<li>При добавлении нескольких fb2 можно запаковать их в один архив для упрощения работы.
<li>При добавлении крупных файлов непакованных форматов (doc, txt, html, etc...) упакуй их зипом. 
<li>При добавлении полученного зипа укажи исходный тип файла (doc, txt, html, etc...). 
<li>При добалении пакованных форматов (djvu, pdf, jpg) упаковывать не обязательно.
</ol>
<ol>Способы
<li><a href=/upload/online/fb2>Добавление книг в формате fb2 (до 2х мегабайт)</a>
<li><a href=/upload/ftp/fb2>Добавление книг в формате fb2 (FTP, любого размера)</a>
<li><a href=/upload/online/>Добавление книг в других форматах (до 2х мегабайт)</a>
<li><a href=/upload/ftp/>Добавление книг в других форматах (FTP, любого размера)</a>
</ol>
";
  case 'ftp':
    set_title("Добавление книг с помощью ftp");
    $r = "1. Загрузи файл на ftp: 
<br>Host: lib.rus.ec
<br>Login: ocr
<br>Password: newbook
<p>2. Заполни форму:
<form action='/upload' enctype='multipart/form-data' method='post'>
<p>Файл <input name=ftpfile size=40>";
    if (arg(2) != 'fb2') {
      $r .= "<p>Тип файла <input size=3 name=filetype> (обязательно)
<p>Название <input size=25 name=title> (обязательно, если не указано внутри fb2)
<p>Жанры, через запятую <input name=genre> (если не указан внутри fb2)
<p>Год издания <input name=year size=4 maxlength=4>
<p>Язык (двухбуквенный код) <input name=lang size=2>
";
    }
    $r .= "<p><input type=submit value='Загрузить'></form>";
    return $r;
    
  case 'online':
    if (arg(2) == 'fb2') 
      return "<form action='/upload' enctype='multipart/form-data' method='post'>
Файл для загрузки с вашего компьютера <input type=file name=book size=80> <input type=submit value='Загрузить'>
<input type=hidden name=filetype value=fb2></form>";

   return "Если автор уже представлен в библиотеке нажми на его страничке кнопку (добавить книгу). 
   Если нет, то заполни форму:".SelectCreateAvtor('a//addbook');
  }
}

function libAvtorAddBook($a) {
  Global $user;
  if (!libcanedit()) return NetPrav;
  $r = "Добавление книги:";
  if ($_FILES['book']['tmp_name']) {
    $fname = trim($_FILES['book']['name']);
    $tname = $_FILES['book']['tmp_name'];
    if (!file_exists("ocr/$fname")) rename($tname,"ocr/$fname");
    $tname = "ocr/$fname";
  } elseif ($_POST['ftpfile']) { 
    $fname = trim($_POST['ftpfile']);
    $tname = "ocr/$fname";
  } else return "<p><p>Загрузка файла в библиотеку. Для массовой загрузки <a href=/upload>удобнее использовать FTP</a>.
<p>Файл будет добавлен в собрание ".avl($a).". Если это неправильно, <a href=/upload/ftp>выберите другого автора</a>
<p>Если у данного произведения несколько авторов, остальных можно будет добавить потом на страничке редактирования книги.
<p>Тип файла может не совпадать с расширением. Если это pdf упакованный в zip, то тип надо ставить pdf.
<p>Если тип файла будет указан как zip или rar, то файл будет распакован и все полученные файлы обработаны по отдельности.
<p>Если несколько fb2 упакованы в один zip и тип указан fb2 (а расширение - zip), то будет проделано аналогично.
<p>Архивы других типов распаковываться не будут.
<p><form action='/a/$a/addbook' enctype='multipart/form-data' method='post'>
<p>Файл для загрузки с вашего компьютера <input type=file name=book size=40> 
<p>Или уже загруженный на ftp либрусека  <input name=ftpfile size=40>
<p>Тип файла <input size=3 name=filetype> (обязательно)
<p>Название <input size=25 name=title> (обязательно, если не указано внутри fb2)
<p>Жанры, через запятую <input name=genre> (если не указан внутри fb2)
<p>Год издания <input name=year size=4 maxlength=4>
<p>Язык (двухбуквенный код) <input name=lang size=2>
<p><input type=submit value='Загрузить'>
</form>";
  $filetype = strtolower ($_POST['filetype']);
  if (preg_match('/\.([^\.]+)$/',$fname,$m)) $ext = $m[1];
  if ($filetype && !$ext) {
    $r .= "<p>Непонятное расширение у файла $fname. Используем $filetype"; 
    $ext=$filetype;
  }
  if (!$filetype && $ext) {
    $r .= "<p>Неуказан тип файла. Используем $ext"; 
    $filetype = $ext;
  }
  if (!$filetype || !$ext) return "Непонятки с типом файла. Перезалейте.";
  $tn1 = translitcyr($tname);
  if (!$tn1) return "Непонятки с названием файла. Перезалейте.";
  if ($tn1 !== $tname && !rename ($tname, $tn1)) 
    return "Ошибка переименования файла.";
  $tname = $tn1;
  $dir = "ocr/Z$user->uid";      
  if (!file_exists($dir)) system ("mkdir '$dir'");
  if ($filetype === 'fb2' || $filetype === 'zip' || $filetype === 'rar') {
    if (preg_match('/\.zip$/',$fname)) $packer = "unzip -jo -d '$dir' '$tname'";
    elseif (preg_match('/\.rar$/',$fname)) $packer = "unrar e -y '$tname' '$dir'";
    else $packer = '';
    if ($packer) {
      $r .= "<hr>".system($packer)."<hr>";
      if ($_POST['filetype'] != 'fb2') $_POST['filetype'] = '';
      unlink ($tname);
      foreach (glob ("$dir/*") as $f) 
        $r .= AddFileToLibrusec($f, 'fb2', '');
      system ("rmdir '$dir'");  
      return $r;
    }  
  }  
  return AddFileToLibrusec($tname, $ext, $a);
}

function libAvtorForum($a, $a1) {
  Global $user;
  $forum = Sel ("nid FROM node_revisions WHERE title like 'A$a %'");
  if ($forum) lgo ("/node/$forum");
  set_title("Обсуждение автора: $a1->aname");

  if (node_access('create', 'forum')) {
    $r = drupal_get_form('forum_node_form', array(
      'uid' => $user->uid, 
      'name' => (isset($user->name) ? $user->name : ''), 
      'type' => 'forum', 
      'language' => 'ru',
      'title' => "A$a $a1->aname",
    ));
    $r = preg_replace('/option value="122"/','option value="122" selected',$r);
    $r = preg_replace('/><.textarea>/', "><a href=/a/$a>$a1->aname</a>\n\n</textarea>",$r);
    return $r;
  }

}

function libAvtorCorrect($a, $a1) {
  return "<h3>Исправление информации об авторе</h3><form action=/a/$a/setname/>
<p>Фамилия: <input name=LastName value=\"$a1->LastName\"> 
Имя: <input name=FirstName value=\"$a1->FirstName\"> 
 Отчество/второе имя: <input name=MiddleName value=\"$a1->MiddleName\"> 
 Прозвище: <input name=NickName value=\"$a1->NickName\"> 
 <input type=submit value='Переименовать автора'></form>";
}

function libAvtorAlias($a) {
 if (!$a2 = arg(3)) 
   return "Вы считаете, что ". avl($a)." является не более, чем ошибочным синонимом для другого имеющегося в библиотеке автора. Давайте исправим ситуацию.<br><br>Поищем кандитатов на должность правильного автора.</br>".SelectAvtor("/a/$a/alias/", "AND AvtorId <> $a");
  $al1 = avl($a,'-',cnt);
  $al2 = avl($a2,'-',cnt);
  return "Вы считаете, что $al1 является не более, чем ошибочным синонимом для $al2<br><a href=/a/$a/doalias/$a2>Произвести замену автора</a> $al1 на $al2<br><br><br><br><br>Вы считаете, что $al2 является не более, чем ошибочным синонимом для $al1<br><a href=/a/$a2/doalias/$a>Произвести замену автора</a> $al2 на $al1<br>";

}

function libAvtorSetName  ($a, $a1) {
  $n1 = trim(strtr ($_GET['FirstName'],'"',"'"));
  $n2 = trim(strtr($_GET['MiddleName'],'"',"'"));
  $n3 = trim(strtr($_GET['LastName'],'"',"'"));
  $n4 = trim(strtr($_GET['NickName'],'"',"'"));
  if (!$n3) 
    return "<p>Фамилия нужна обязательно. Нельзя никак без фамилии";
  if ($a->FirstName === $n1 && $a->SecondName === $n2 &&$a->LastName === $n3)
    return "<p>Похоже, Ничего не изменилось";
  if ($aid=Sel("AvtorId FROM libavtorname WHERE FirstName=\"$n1\" AND MiddleName=\"$n2\" AND LastName=\"$n3\" AND AvtorId != $a")) 
    return "<p>Такой автор уже есть - ".avl($aid).
           " <a href=/a/$a/alias/$aid/>(создать синоним)</a>";
   
  LogAction("UPDATE libavtorname SET FirstName=\"$n1\", MiddleName=\"$n2\", LastName=\"$n3\", NickName=\"$n4\" WHERE AvtorId=$a LIMIT 1", 
        "Rename avtor $a", "UPDATE libavtorbook SET FirstName=\"$a1->FirstName\", MiddleName=\"$a1->MiddleName\", LastName=\"$a1->LastName\", NickName=\"$a1->NickName\" WHERE AvtorId=$a");  
  return lgo("/a/$a/");
}										

function libAvtorDoAlias($a1) {
  $a2 = arg(3);
  if (!$a2) return libAvtorAliase($a1);		
  if ($a != $a2) $r = DoAvtorAlias($a1, $a2);
  return $r.libAvtorSearchd($a2);
}

function DoAvtorAlias ($bi, $gi) {
  Global $user;
  if (Sel ("* FROM libavtoraliase WHERE BadId = $gi")) 
    return "<p>Товарищь ".avl($gi)." сам алиас. Так не пойдёт";
  $st = SELECT ("BookId FROM libavtor WHERE AvtorId = $bi");
  while ($a1 = dbf($st)) {
    $bn++;
    $b = $a1->BookId; 
    $id = $a1->ID;
    LogAction("UPDATE libavtor SET AvtorId=$gi WHERE AvtorId = $bi AND BookId = $b", 
              "Change Avtor $bi to $gi for book $b",
              "UPDATE libavtor SET AvtorId=$bi WHERE AvtorId = $gi AND BookId = $b");
  }
  LogAction("INSERT INTO libavtoraliase (BadId, GoodId) values ($bi, $gi)",
             "Avtor Alias $bi to $gi", "DELETE FROM libavtoraliase WHERE BadId=$bi AND GoodId=$gi");
  $r .= "<p>Обработано $bn книг. Спасибо за помощь";
  if ($nid = Sel ("nid FROM node_revisions JOIN node USING (nid) WHERE node_revisions.title = '$bi' AND type='adesc'")) 
    if (!Sel("nid FROM node_revisions JOIN node USING (nid) WHERE node_revisions.title = '$gi' AND type='adesc'"))
      Update ('node_revisions', "nid = $nid", "title = '$bi'");
  return $r;
}

function joinbooks($b1, $b2, $tit = '') {
  libBookDoJoin($b1, $b2);
  return " И удалили его. (<a href=/b/$b2/undel> восстановить $tit</a>)";
}
  
function libAvtorSearchd($a) {
  $st = SELECT("Sum(1) as s, SUBSTRING_INDEX(Title, '(',1) as T from libavtor join libbook using (bookid) where AvtorId = $a AND NOT (Deleted&1) group by 2");
  $r .= "<ol>";
  while ($a1 = dbf($st)) {
    if ($a1->s < 2) continue;
    $tit = rtrim($a1->T);
    $st1 = SELECT ("libbook.Id AS Id, Ver, FileAuthor, Title, BookId AS b, FileSize, FileType FROM libbook JOIN libavtor USING(BookId) WHERE Title LIKE \"$tit%\" AND AvtorId = $a AND NOT (1&Deleted) LIMIT 2");
    $a2 = dbf($st1); $b = $a2->b; 
    $a3 = dbf($st1); $b2 = $a3->b;
    if ($b == $b2) continue;
    ++$dubliii;
    $r .= "<li>Кажется, нашли дубля.";
    if ($a2->FileType === 'fb2' && $a3->FileType === 'fb2') {
      $v1 = 100*GetVer($b);      
      $v2 = 100*GetVer($b2);     
      $r .= " $v1:$v2 ";
      if ($v1 > $v2 && $v2 > 0 && $a2->Id == $a3->Id) 
        $r .= joinbooks($b, $b2, $a3->Title);
      elseif ($v2 > $v1 && $v1 > 0 && $a2->Id == $a3->Id) 
        $r .= joinbooks($b2, $b, $a3->Title);
      if ($a2->FileAuthor == 'rusec') 
        $r .= joinbooks($b2, $b, $a3->Title);
      elseif ($a3->FileAuthor == 'rusec') 
        $r .= joinbooks($b, $b2, $a3->Title);
/*
      if ($a2->FileSize < 1.1*$a3->FileSize && !ReadAnnotation($b, $nid) || $v1 < $v2) {
        $r .= " И удалили его. (<a href=/b/$b/undel> восстановить $tit</a>)";
        DeleteBook($b);
        continue;
      }      
      if ($a2->FileSize*1.1 > $a3->FileSize && !ReadAnnotation($b2, $nid) || $v2 < $v1) {
         $r .= " И удалили его. (<a href=/b/$b2/undel> восстановить $tit</a>)";
         DeleteBook($b2);
         continue ;
      }      
*/      
    }
    $r .= "Ткните для выбора.";
    $r .= " <a href=/b/$b/join/$b2/ target=_blank>$tit</a>\n";
    $d1 = $b; $d2=$b2;
  }
  if ($dubliii == 1) return lgo("/b/$d1/join/$d2");
  return $r . "</ol><br><a href=/a/$a/edit>Продолжить обработку</a> автора ".avl($a);
}

function AddFileToLibrusec($f, $ext, $a) {
  global $user; 
  if (!file_exists($f)) return "<p>Не найден файл $f. Беда...";
  $GenreIds = array();  
  $filetype = strtolower ($_POST['filetype']);
  if (!$filetype) $filetype = $ext;
  $lang = strtolower ($_POST['lang']);
  $year = $_POST['year'];
  $title = $_POST['title'];
  if (!$title && $filetype == 'fb2') 
    $title = 'from-fb2';
  if (!$ext || !$title)
    if (preg_match('/([^\/]+)\.([^\.]+)$/',$f,$m)) {
      $fil = $m[1];
      $ext = $m[2];
      $r .= "fil=$fil;";
    } else {
      $r .= "<p>Непонятное расширение. Используем $filetype"; 
      $ext=$filetype;
    }
  if (!$filetype) $filetype = strtolower($ext);
  if (!$title) {
    $title = $fil;
//    $title = iconv ("CP1251", "UTF-8", $fil);
    $r .= "Title: $title";
  }  
  $r .= "<form method=post action=/a/$a/addfile target=_blank><hr>Добавленине книги $title.<br>";
  if ($filetype == 'fb2') {
    if ($e = parsefb2($f, 0, 'head')) return $e;
    Global $PD;
    if ($tt = $PD->head['FICTIONBOOK/DESCRIPTION/TITLE-INFO/BOOK-TITLE'])
      $title = $tt;
    else {print "В FB2 не найдено название книги. Такой FB2 нам не нужен"; exit;}
    $genre = $PD->head['FICTIONBOOK/DESCRIPTION/TITLE-INFO/GENRE'];  
  }
  if (!$genre) $genre = strtolower($_POST['genre']);
//  Global $debug; $debug = 1;
  $GNR = array(architecture=>design,art=>design,art_instr=>design,artists=>design,fashion=>design,graph_design=>design,performance=>dramaturgy,photography=>design,people=>nonf_biography,biography=>nonf_biography,biogr_arts=>nonf_biography,biogr_ethnic=>nonf_biography,biogr_family=>nonf_biography,biogr_historical=>nonf_biography,biogr_leaders=>nonf_biography,biogr_professionals=>nonf_biography,biogr_sports=>nonf_biography,biogr_travel=>nonf_biography,business=>sci_business,biz_accounting=>sci_business,biz_beogr=>nonf_biography,biz_life=>sci_business,biz_careers=>sci_business,biz_economics=>sci_business,biz_finance=>sci_business,biz_international=>sci_business,biz_professions=>sci_business,biz_investing=>sci_business,biz_management=>sci_business,biz_sales=>sci_business,biz_personal_fin=>sci_business,biz_ref=>sci_business,biz_small_biz=>sci_business,child_3=>child_tale,child_4=>child_prose,child_9=>child_prose,child_animals=>adv_animal,child_art=>children,child_computers=>computers,child_edu=>child_education,child_history=>child_prose,child_obsessions=>children,child_people=>children,child_characters=>child_prose,child_ref=>children,child_religion=>religion_rel,child_nature=>child_education,child_series=>children,child_sports=>children,chris_bibles=>religion_rel,chris_pravoslavie=>religion_rel,chris_catholicism=>religion_rel,chris_living=>religion_rel,chris_history=>religion_rel,chris_nlergy=>religion_rel,chris_edu=>religion_rel,chris_evangelism=>religion_rel,chris_fiction=>religion_rel,chris_holidays=>child_prose,chris_jesus=>religion_rel,chris_mormonism=>religion_rel,chris_orthodoxy=>religion_rel,chris_protestantism=>religion_rel,chris_ref=>religion_rel,chris_theology=>religion_rel,chris_devotion=>religion_rel,compusers=>computers,comp_office=>computers,comp_cert=>computers,comp_games=>computers,comp_sci=>computers,comp_db=>comp_db,comp_biz=>computers,comp_graph=>computers,comp_hardware=>comp_hard,comp_microsoft=>computers,comp_networking=>comp_osnet,comp_os=>comp_osnet,comp_programming=>comp_programming,comp_software=>comp_soft,comp_www=>comp_www,cooking=>home_cooking,cook_baking=>home_cooking,cook_can=>home_cooking,cook_art=>home_cooking,cook_drink=>home_cooking,cook_gastronomy=>home_cooking,cook_meals=>home_cooking,cook_natural=>home_cooking,cook_outdoor=>home_cooking,cook_pro=>home_cooking,cook_quick=>home_cooking,cook_ref=>home_cooking,cook_regional=>home_cooking,cook_appliances=>home_cooking,cook_diet=>home_cooking,cook_spec=>home_cooking,cook_veget=>home_cooking,entertainment=>home_entertain,entert_comics=>home_entertain,entert_games=>home_entertain,entert_humor=>home_entertain,entert_movies=>home_entertain,entert_music=>home_entertain,performance=>dramaturgy,nonfiction_pop_culture=>home_entertain,entert_radio=>home_entertain,entert_tv=>home_entertain,gay_biogr=>nonf_biography,history_gay=>nonf_biography,literature_gay=>prose_counter,gay_mystery=>sf_horror,gay_nonfiction=>nonfiction,gay_parenting=>home,gay_travel=>adv_geo,health=>home_health,health_aging=>sci_medicine,health_alt_medicine=>sci_medicine,health_beauty=>home_health,health_cancer=>sci_medicine,health_death=>home_health,health_diets=>home_health,health_dideases=>home_health,health_fitness=>home_health,health_men=>home_health,health_mental=>sci_psychology,health_nutrition=>home_health,health_personal=>home_health,health_psy=>sci_psychology,health_recovery=>home_health,health_ref=>home_health,health_rel=>home_health,health_first_aid=>home_health,health_self_help=>home_health,health_sex=>home_sex,health_women=>home_health,history_africa=>sci_history,history_america=>sci_history,history_ancient=>sci_history,history_asia=>sci_history,history_australia=>sci_history,history_europe=>sci_history,history_study=>sci_history,history_jewish=>sci_history,history_middle_east=>sci_history,histor_military=>sci_history,history_military_science=>sci_history,history_russia=>sci_history,history_usa=>sci_history,history_world=>sci_history,history_gay=>sci_history,home_pets=>home_pets,home_collect=>home_crafts,home_crafts=>home_crafts,home_entertain=>home_entertain,home_expert=>home_diy,home_garden=>home_garden,home_design=>home_diy,home_howto=>home_diy,home_interior_design=>home_diy,home_cottage=>home,home_weddings=>home,horror=>sf_horror,horror_antology=>sf_horror,horror_british=>sf_horror,horror_fantasy=>sf_horror,horror_erotic=>sf_horror,horror_ghosts=>sf_horror,horror_graphic=>sf_horror,horror_occult=>sf_horror,horror_ref=>sf_horror,horror_usa=>sf_horror,horror_vampires=>sf_horror,literature=>prose_classic,literature_books=>prose_classic,literature_british=>prose_classic,literature_classics=>prose_classic,literature_drama=>prose_classic,literature_essay=>prose_classic,literature_adv=>adventure,literature_antology=>prose_classic,literature_erotica=>love_erotica,literature_fairy=>child_tale,literature_saga=>prose_classic,literature_gay=>prose_counter,literature_history=>prose_history,literature_men_advent=>adventure,literature_political=>prose_contemporary,literature_religion=>religion_rel,literature_sea=>adv_maritime,literature_sports=>home_sport,literature_war=>prose_contemporary,literature_western=>adv_western,literature_critic=>prose_history,literature_letters=>nonf_biography,literature_poetry=>poetry,literature_short=>prose_classic,literature_usa=>prose_classic,literature_women=>love_contemporary,literature_world=>prose_classic,literature_rus_classsic=>prose_rus_classic,literature_su_classics=>prose_su_classics,mystery=>det_history,thriller_mystery=>detective,thriller_police=>det_police,thriller=>thriller,nonfiction_avto=>nonfiction,nonfiction_crime=>nonfiction,nonfiction_events=>nonfiction,biz_economics=>sci_business,nonfiction_edu=>science,nonfiction_gov=>science,nonfiction_holidays=>science,nonfiction_law=>sci_juris,nonfiction_philosophy=>sci_philosophy,nonfiction_politics=>nonfiction,nonfiction_social_sci=>science,nonfiction_antropology=>sci_history,science_archaeology=>sci_history,nonfiction_traditions=>nonfiction,nonfiction_demography=>nonfiction,nonfiction_racism=>nonfiction,nonfiction_emigration=>nonfiction,nonfiction_ethnology=>science,nonfiction_folklor=>antique_myths,nonfiction_gender=>science,nonfiction_gerontology=>science,nonfiction_hum_geogr=>science,nonfiction_methodology=>science,nonfiction_philantropy=>nonfiction,nonfiction_pop_culture=>nonfiction,nonfiction_pornography=>home_sex,nonfiction_ref=>reference,nonfiction_research=>science,nonfiction_social_work=>science,nonfiction_sociology=>science,nonfiction_spec_group=>science,nonfiction_stat=>science,health_psy=>sci_psychology,nonfiction_transportation=>nonfiction,nonfiction_true_accounts=>nonfiction,nonfiction_urban=>nonfiction,nonfiction_women=>nonfiction,outdoors_birdwatching=>sci_biology,outdoors_conservation=>nonfiction,outdoors_ecology=>sci_biology,outdoors_ecosystems=>sci_biology,outdoors_env=>sci_biology,outdoors_fauna=>sci_biology,outdoors_field_guides=>ref_guide,outdoors_flora=>sci_biology,outdoors_hiking=>home_crafts,outdoors_hunt_fish=>home_crafts,outdoors_resources=>science,outdoors_nature_writing=>sci_biology,outdoors_outdoor_recreation=>home_sport,outdoors_ref=>sci_biology,outdoors_survive=>home_sport,outdoors_travel=>adv_geo,family=>home,family_adoption=>home,family_aging_parents=>home,family_edu=>home,family_activities=>home,family_health=>home_health,family_relations=>home,family_fertility=>home_health,family_humor=>humor,family_lit_guide=>home,family_parenting=>home_health,family_pregnancy=>home_health,family_ref=>reference,family_special_needs=>home_health,professional_finance=>sci_business,architecture=>design,professional_management=>sci_business,professional_edu=>science,professional_enginering=>sci_tech,professional_law=>sci_juris,professional_medical=>sci_medicine,professional_sci=>sci_tech,references=>reference,ref_almanacs=>ref_ref,biz_ref=>sci_business,ref_careers=>ref_ref,ref_catalogs=>ref_ref,ref_cons_guides=>ref_ref,ref_dict=>ref_ref,ref_edu=>sci_business,ref_encyclopedia=>ref_encyc,ref_etiquette=>reference,ref_langs=>reference,ref_fun=>reference,ref_genealogy=>sci_history,nonfiction_law=>sci_juris,ref_books=>reference,ref_quotations=>reference,ref_study_guides=>ref_ref,ref_words=>reference,ref_writing=>prose_contemporary,religion=>religion_rel,religion_bibles=>religion_rel,religion_buddhism=>religion,Christianity=>religion_rel,religion_earth=>religion,religion_fiction=>religion_rel,religion_hinduism=>religion,religion_islam=>religion,religion_judaism=>religion,religion_new_age=>religion_rel,religion_occult=>religion_esoterics,religion_religious_studies=>religion_rel,religion_spirituality=>religion_esoterics,religion_east=>religion_self,religion_other=>religion,romance=>prose_classic,romance_anthologies=>prose_classic,romance_contemporary=>prose_contemporary,romance_fantasy=>sf_fantasy,romance_sf=>sf,romance_historical=>prose_history,romance_multicultural=>prose_contemporary,romance_regency=>prose_classic,romance_religion=>religion_rel,romance_romantic_suspense=>love_contemporary,romance_series=>prose_contemporary,romance_time_travel=>sf,ref_writing=>prose_contemporary,science=>science,science_agri=>science,science_archaeology=>sci_history,science_astronomy=>science,science_behavioralsciences=>sci_psychology,science_psy=>sci_psychology,science_biolog=>sci_biology,science_chemistry=>sci_chem,science_earth=>science,science_edu=>science,science_evolution=>science,science_measurement=>science,science_history_philosophy=>sci_history,science_math=>sci_math,science_medicine=>sci_medicine,science_eco=>science,science_physics=>sci_phys,science_ref=>science,science_technology=>sci_tech,romance_sf=>sf,romance_fantasy=>sf_fantasy,gaming=>sf,sf_writing=>sf,fantasy_alt_hist=>sf_history,sf_cyber_punk=>sf_cyberpunk,sport=>home_sport,teens_beogr=>nonf_biography,teens_health=>home_sport,teens_history=>adv_history,teens_horror=>sf_horror,teens_literature=>prose_classic,teens_mysteries=>detective,teens_ref=>reference,teens_religion=>religion_rel,teens_school_sports=>home,teens_sf=>sf,teens_tech=>science,teens_series=>child_adv,teens_social=>sci_psychology,travel=>adv_geo,travel_africa=>adv_geo,travel_asia=>adv_geo,travel_australia=>adv_geo,travel_canada=>adv_geo,travel_caribbean=>adv_geo,travel_europe=>adv_geo,travel_guidebook_series=>adv_geo,travel_lat_am=>adv_geo,travel_middle_east=>adv_geo,travel_polar=>adv_geo,travel_spec=>adv_geo,travel_usa=>adv_geo,travel_rus=>adv_geo,travel_ex_ussr=>adv_geo,women_divorce=>home,women_domestic=>home,women_child=>home,women_single=>home);
  foreach (split (',' , $genre) as $gn) {
    if ($GNR[$gn]) $gn = $GNR[$gn];
    if ($g = Sel("GenreId FROM libgenrelist WHERE GenreId = '$gn' OR GenreCode = '$gn' OR GenreDesc = '$gn'"))
      $GenreIds[] = $g;
    else return "непонятный жанр $gn";
  }    
  $r .= "<hr>";
  if (!$GenreIds[0]) {
    $r .= "<p>Жанр не указан или указан неверно. Поправьте: <input name=genre value='$genre'>";
    $err++;
  } else $r .= "<input type=hidden name=genre value=\"$genre\"> ";
  if (!$title) {
    $r .= "<p>Не указано название книги. Поправьте: <input name=title>";
    $err++;
  } else $r .= "<input type=hidden name=title value=\"$title\"> ";
  if (!$filetype) {
    $r .= "<p>Не указан тип файла. Поправьте: <input name=filetype size=4>";
    $err++;
  }  else $r .= "<input type=hidden name=filetype value=\"$filetype\">";
  if ($err) return $r . "<p><input type=hidden name=filename value=\"$f\"><p>Год издания <input name=year size=4 maxlength=4 value=$year><p>Язык (двухбуквенный код) <input name=lang value='$lang' size=2><p><input type=submit></form><hr>\n\n";   
  if (!$title || $title == 'from-fb2') return $r . "<br>Название не найдено";

// всё хорошо, добавляем книгу.
  $fs = filesize($f); 
  if ($PD->AvtorIds[0]) $a = $PD->AvtorIds[0]; else $PD->AvtorIds[0] = $a;
  $r .= " Наменование: $title<br>";
//Попытаемся выловить дубли. Может, получится. Будем надеяться. Дубли - это наша беда. Да.
  $t = trim(preg_replace('/\(.+\d+\s*\)/','',$title));
  $t = preg_replace('/…/','',$t); 
  $t = preg_replace('/"/',"'",$t); 
//  print " Наменование: $t";
//if ($user->uid == 1) {print "<pre>"; print_r($PD); exit;}
  $id = $PD->head['FICTIONBOOK/DESCRIPTION/DOCUMENT-INFO/ID'];
  if ($id && ($BookId = Sel ("BookId FROM libbook WHERE Id = '$id'"))) {
    $oldver = Sel ("Ver FROM libbook WHERE BookId = $BookId");
    $ver = $PD->head['FICTIONBOOK/DESCRIPTION/DOCUMENT-INFO/VERSION'];
    if (1*$oldver >= 1*$ver) {
      unlink ($f);
      return $r . " Версия не лучше: ".bl($BookId)." $oldver:$ver<br>";
    }
  } 
  foreach($PD->AvtorIds as $aid) {
    if ($BookId = Sel ("libbook.BookId FROM libbook JOIN libavtor USING (BookId) WHERE Title=\"$t\" AND AvtorId = '$aid' AND NOT (1&Deleted)")) break;   
    $ln = Sel ("LastName FROM libavtorname WHERE AvtorId = $aid");
    $r .= "Проверяем $ln<br>";
    if ($BookId = Sel ("libbook.BookId FROM libbook JOIN libavtor USING (BookId) JOIN libavtorname USING(AvtorId) 
WHERE Title LIKE \"$t%\" AND FileType = '$filetype' AND LastName = \"$ln\" AND NOT (1&Deleted) LIMIT 1")) break;  
  }
  if ($a == 643) $avtors = 'VA'; 
  else $avtors = AvtorName($a);
  $outname = str_replace('/','_',translitcyr($avtors.' '.$title));
  $r .= " file:$outname<br>";
  $fff = "b/$outname.fb2";
  if ($BookId) {
    if ($filetype !== 'fb2') {
      unlink ($f);
      return $r . "Такая книга уже есть. Что-то не так. Обратитесь в администрацию<br>";
    } elseif ($fs == Sel("FileSize FROM libbook WHERE BookId = $BookId")) {
      unlink ($f);
      return "<br>Книга уже есть в библиотеке - ".bl($BookId); 
    }  
  }    
// Не дубль, во всяком случае не для шарка. Добавляем по-взрослому.

  $ii = '';
  $aname = '';
  if ($a != 643) $aname = translitcyr(AvtorName($a));
  if ($aname) $aname = $aname.'_';
  do {
    $nn = str_replace('/','_',$aname.translitcyr($title).$ii);
    if ($filetype == 'fb2') $nn1 = "b/$nn.fb2";
    else {
      $nn = "$nn.$ext";
      $nn1 = "b.usr/$nn";
    }  
    $ii++;
  } while (file_exists($nn1));
  rename ($f, $nn1); 
  if (!file_exists($nn1)) return $r. "<p>Не получилось записать файл $f в $nn1";
  $year = $year>0?(integer)$year:$PD->head['FICTIONBOOK/DESCRIPTION/PUBLISH-INFO/YEAR'];;
  if (!$lang) $lang = $PD->head['FICTIONBOOK/DESCRIPTION/TITLE-INFO/LANG'];
  if ($filetype != 'fb2') {
    if ($qq=Sel("BookId FROM libfilename WHERE FileName = \"$nn\"")) 
      return $r. "<p>Файл $nn <a href=/b/$qq>уже есть в базе</a>. Повторы недопустимы.";
  } else {
    $nn = Sel("MAX(BookId) FROM libbook") + 1 + $user->uid*100;
  }    
  if (!$GenreIds[0]) return $r . "<br>Жанр не найден";
  $title = addslashes($title);
  if ($p = $PD->head['FICTIONBOOK/DESCRIPTION/DOCUMENT-INFO/AUTHOR/NICKNAME']) $nick = $p;
  else {
    $nick = $PD->head['FICTIONBOOK/DESCRIPTION/DOCUMENT-INFO/AUTHOR/FIRST-NAME'].' '. $PP->head['FICTIONBOOK/DESCRIPTION/DOCUMENT-INFO/AUTHOR/LAST-NAME'];
    if (strlen($nick) < 3) $nick = $PD->head['FICTIONBOOK/DESCRIPTION/DOCUMENT-INFO/AUTHOR/PROGRAM-USED'];
  }
  $nick = addslashes($nick);
  $keywords = addslashes($PD->head['FICTIONBOOK/DESCRIPTION/TITLE-INFO/KEYWORDS']);
  $srclang = addslashes($PD->head['FICTIONBOOK/DESCRIPTION/TITLE-INFO/SRC-LANG']);
  $Action = "INSERT INTO libbook (FileSize, FileType, Title, Year, Id, Ver, Lang, FileAuthor, keywords) VALUES ('$fs', '$filetype','$title','$year', '$id', '$ver', '$lang', '$nick', '$keywords')";
  LogAction ($Action, "Add book","");
  $actionid = Sel ("ActionId FROM libactions WHERE ActionDesc = 'Add book' AND ActionSQL = \"$Action\" ORDER BY ActionId DESC LIMIT 1" );
  watchdog('librusec', "SELECT ActionId FROM libactions WHERE ActionDesc = 'Add book' AND ActionSQL = \"$Action\" ORDER BY ActionId DESC LIMIT 1");
  if (!$actionid) $actionid = Sel("ActionId FROM libactions WHERE ActionDesc = 'Add book' AND ActionSQL like '%$fs%$id%' ORDER BY ActionId DESC LIMIT 1");
  $b = Sel("BookId FROM libbook WHERE Title = '$title' AND FileSize = '$fs' ORDER BY BookId DESC LIMIT 1");
  if (!$b) return $r. "Что-то не получилось. Свяжитесь с администрацией";
  system ("chmod a+r \"$nn1\"");
  if ($filetype == 'fb2') {
    rename ($nn1, "b/$b.fb2"); 
  } else {
    Insert (libfilename, "BookId, FileName", "$b, \"$nn\"");
  }  
  Update (libactions, "BookId=$b", "ActionId=$actionid");
  $r .= "<p><a href=/b/$b/edit target=_blank>Книга добавлена</a><br>";
  foreach ($GenreIds as $gid) Insert ('libgenre', 'BookId, GenreId', "$b, $gid"); 
  if ($PD->AvtorIds) foreach ($PD->AvtorIds as $aid) Insert ('libavtor', 'BookId, AvtorId', "$b, $aid"); 
  if ($PD->TranslatorIds) foreach ($PD->TranslatorIds as $aid) Insert ('libtranslator', 'BookId, TranslatorId', "$b, $aid"); 
  if ($PD->SeqIds) foreach($PD->SeqIds as $seq) Insert (libseq, 'BookId, SeqId, SeqNumb, Level', "$b, $seq[0], '$seq[1]', $seq[2]");
  
  if ($dublid = Sel ("BookId FROM libbook JOIN libavtor USING (BookId) WHERE Title=\"$title\" AND AvtorId = $a AND BookID != $b AND NOT (Deleted&1)")) {
     $r .= "<p>Кажется, нашли дубля.";
     if (user_access('библиотекарь')) {
           //
           // $b      - new book Librusec-book-number, not the FICTIONBOOK/DESCRIPTION/DOCUMENT-INFO/ID
           // $dublid - dubl Librusec-book-number
           // $id  - new book document-info-ID (was set earlier)
           // $ver - new book document-info-version (was set earlier)
           //
           // Get document-info-ID and version for dubl
           $dubldocid = Sel ("Id FROM libbook WHERE BookId = $dublid"); // ya ne uveren v sintaksise
           $dublver =  Sel ("Ver FROM libbook WHERE BookId = $dublid");
           // Automatic upgrade of the same book
           if( ($dubldocid==$id) && (1.0*$ver > 1.0*$dublver) ){
               $r .= joinbooks($b,$dublid,$title);
               return $r . "<hr>\n";
           };
           // Automatic upgrade of the old "LibRusEc kit" book
           //                            012345678901234567890123
           // U libruseka dva tipa id - "Tue Jun 12 03:34:15 2007" i "Mon Jun 10 19:57:46 2013"
           // Kljuchevoe - probely s godom.
           $i = strrpos($dubldocid," 20");
           $r .= " ($i)($dubldocid) "; // debugonly
           if( ($i==19) && (1.0*$ver > 1.0*$dublver) ){
               $r .= joinbooks($b,$dublid,$title);
               return $r . "<hr>\n";
           };
     }; // if библиотекарь
     // No automatic resolution, resolve conflict manually
     $r .= " Ткните в него для разрешения конфликта. <a href=/b/$b/join/$dublid  target=_blank>Дубль: $title</a>";  
  }
  return $r . "<hr>\n";
}

function libAvtorRSS($a) {
  if ($lang = arg(3))
    return libRSS("a/$a.$lang", 24, AvtorName($a), "a/$a/$lang", "JOIN libavtor USING (BookId)", "AvtorId = $a AND Lang = '$lang' AND");
  else  
    return libRSS("a/$a", 24, AvtorName($a), "a/$a", "JOIN libavtor USING (BookId)", "AvtorId = $a AND");
}

function AllAvtors() {
  $CacheFile = 'cache/AllAvtorList';
  if ($fh = libOpenCacheFile($CacheFile, 24*3600)) {
    $sth = SELECT ("AvtorId, CONCAT_WS(' ', LastName, FirstName, MiddleName) AS Name, SUM(1) AS N 
    FROM libavtorname JOIN libavtor USING (AvtorId) JOIN libbook USING (BookId) 
    WHERE NOT (Deleted&1)
    GROUP BY 1,2
    ORDER BY 2");
    while ($a1 = dbf($sth)) fwrite($fh, "<li><a href=/a/$a1->AvtorId>$a1->Name ($a1->N)</a>\n");
    fclose($fh);
  }
  return "<ol>".libReadCacheFile ($CacheFile)."</ol>";
}
									
function libAvtorStat($a, $c1, $c2, $c3, $order) {
  $ua = $_SERVER['HTTP_USER_AGENT'];
  if (strstr($ua, 'Yandex') or strstr($ua, 'Google') or strstr($ua, 'bot') ) return '';

  $r = "<form action=/a/$a/stat>Сортировать";
  $r .= '<input type=radio name=order value=a '.$c1.' onClick="submit()">По алфавиту 
<input type=radio name=order value=b onClick="submit()" '.$c2.'>По сериям 
<input type=radio name=order value=t onClick="submit()" '.$c3.'>По дате поступления<br></form>';

  $mname = array ("Дек", "Янв", "Фев", "Мар", "Апр", "Май", "Июнь", "Июль", "Авг", "Сен", "Окт", "Ноя", "Дек");
  $sth = SELECT("BookId, Mes, libstat.N AS N, Deleted 
                 FROM libstat JOIN libavtor USING(BookId) JOIN libbook USING(BookId) LEFT JOIN libseq USINF(BookId) LEFT JOIN(libseqname) USING (SeqId)
                 WHERE AvtorId = $a ORDER BY $order");
  
  while ($a1 = dbf($sth)) {
    $dl = $a1->Deleted&1;
    $b = $a1->BookId;
    $Deleted[$b] = $dl;
    $n = $a1->N;
    $ym = $a1->Mes*1;
    while ($dl & 1) {
      if ($b1 = Sel ("GoodId FROM libjoinedbooks WHERE BadId = $b")) 
        $dl = Sel ("Deleted FROM libbook WHERE BookId = ".($b = $b1));
      else 
        $dl = 0;
    }
    $ALL[$b] += $n; 
    $ALLM[$ym] += $n;
    $BM[$b][$ym] += $n;
    $all += $n;
    if (!$firstym || $ym < $firstym) $firstym = $ym;
    if (!$lastym || $ym > $lastym) $lastym = $ym;
  }
  $r .= "\n<table><tr><th><th>Всего";
  for ($i = $firstym; $i <= $lastym; $i++) 
    if ($ALLM[$i]) 
      $r .= "<th>".$mname[$i%100];
  $r .= "\n<tr><th>";
  $r .= avl($a);
  $r .= "<th>$all";
  for ($i = $firstym; $i <= $lastym; $i++) if ($ALLM[$i]) 
    $r .= "<th>$ALLM[$i]</a>";
  foreach ($ALL as $b => $n) {
    $newseq = Sel("SeqId FROM libseq WHERE BookId = $b");
    if ($c2 && $seq != $newseq) {
      if ($seq = $newseq) $seqname = Sel("SeqName FROM libseqname WHERE SeqId = $seq");
      else $seqname = "Вне сериалов";
      $r .= "<tr><th>$seqname";
    }
    if ($seq) $seqn = Sel("SeqNumb FROM libseq WHERE BookId = $b"). ". ";
    else $seqn = '';
    $r .= "\n<tr><td>$seqn".bl($b);
    if ($Deleted[$b]&1) $r .= " (удалена)";
    $r .="<td>".$ALL[$b];
    for ($i = $firstym; $i <= $lastym; $i++) if ($ALLM[$i]) $r .= "<td>".$BM[$b][$i];
  }
  $r .= "</table>\n\n";
  return $r;
}																				

function libAvtorDeleteAlias($a) {
  $a2 = arg(3);
  if (user_access('библиотекарь') && $a2 > 0) 
    LogAction("DELETE FROM libavtoraliase WHERE GoodId = $a AND BadId = $a2", 
    "Delete Alias $a2 to $a", 
    "INSERT INTO libavtoraliase(GoodId, BadId) VALUES ($a, $a2)");
  lgo("/a/$a/edit");
}

																															
