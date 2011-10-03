<?php
include_once drupal_get_path('module', 'librusec') .'/librusec.inc';

function libBookLib() {
  $f = arg(1);
  if ($f && $f != 'search') {
    if (file_exists("book.lib/books/$f")) 
      drupal_goto("book.lib/books/$f");
    if (!$a1 = S("* FROM booklib WHERE Filename LIKE '".addslashes($f)."%'"))
      return "File $f not found in booklib";
    drupal_goto("http://book.lib.rus.ec/books/$a1->Filename");    
  }

  if ($aj = $_GET['aj']) {//ajax call
    Global $PP, $user; $PP=20;
    $W = '';
    if ($t = p('t')) {$t=str_replace('*','%',$t); $W .= " AND Title LIKE CONVERT(_utf8 '$t%' USING cp1251)";}
    if ($t = p('fn')) $W .= " AND (A1N1 LIKE CONVERT(_utf8 '$t%' USING cp1251) OR A2N1 LIKE CONVERT(_utf8 '$t%' USING cp1251))";
    if ($t = p('ln')) $W .= " AND (A1N2 LIKE CONVERT(_utf8 '$t%' USING cp1251) OR A2N2 LIKE CONVERT(_utf8 '$t%' USING cp1251))";
    if ($t = p('g'))  $W .= " AND (G1 LIKE CONVERT(_utf8 '%$t%' USING cp1251) OR G2 LIKE CONVERT(_utf8 '%$t%' USING cp1251) OR G3 LIKE CONVERT(_utf8 '%$t%' USING cp1251))";
    if ($t = p('s')) $W .= " AND Seq LIKE CONVERT(_utf8 '%$t%' USING cp1251)";
    if ($t = p('f')) $W .= " AND FileName LIKE '$t%'";
    if ($W) $W = "WHERE 1 $W";
    $sth = pgq("SELECT * FROM booklib $W ORDER BY Title", Sel("count(*) FROM booklib $W"));
    while ($a1 = dbf($sth)) print printline($a1);
    print theme('pager');
    exit;
  }

  $ask = $_GET['a'];
  str_replace ('"','',$ask);
  $r .= "<h2>Поиск по файл-эхе SU.BOOKS</h2>
Полностью или большими кусками можно выкачивать с <a href=http://book.lib.rus.ec>http://book.lib.rus.ec</a>. Тут можно найти и выкачать конкретный файл.<br>
С общей коллекцией либрусека эта часть никак не связана. Это зеркало book-libа.<br> 
Если тут попадётся что-то, отсутствующее в основной коллекции, просьба перегнать в fb2 и залить. Спасибо.
";
  if ($ask) {
    $r .= "<h4>Ищем: $ask</h4>";
    $l = "LIKE CONVERT(_utf8 '$ask%' USING cp1251)";
    $W = "Title $l OR A1N1 $l OR A1N2 $l OR A2N1 $l OR A2N2 $l OR Seq $l";
    $sth = pgq("SELECT * FROM booklib WHERE $W", Sel ("count(*) FROM booklib WHERE $W"));
    while ($a1=dbf($sth)) $r .= printline($a1);
  } else {
  $r .= "
<script>
var page='$_GET[page]';
var lp = '1';
var ltm = 0;
var lp1 = '';
var cnt=0;
function RB(now) {
 var tm = new Date; tm = tm.getTime();
 if (now != 1 && tm - ltm < 1111) { setTimeout(RB, 555); return; } //были недавно, зайдите попозже.
 ltm = tm;
 var par = '';
 if (page) par = 'page='+page+'&';
 var a = ['t', 'g', 'ln', 'fn', 's', 'f'];
 for (var i in a) {
   var q = $('#'+a[i]).val();
   if (q) par = par + a[i] + '=' + encodeURI(q) + '&';   
 }  
 if (now != 1 && lp1 != par) {//прошлый раз просили не это. Еще не определились. Зайдите попозже.
   lp1 = par;
   setTimeout(RB, 1111);
   return;
 }
 if (!par) {
   $('#books').html('');
   return;
 }  
 if (lp != par) {
   $('#books').html('<p>грузим...</p>');
   jQuery.get('http://lib.rus.ec/booklib?aj=ax&'+par, function(r){ 
   r=r.replace(/..booklib\?page=(\d+)[^\"]*\"/g, 'javascript:pg($1)');
   r=r.replace(/..booklib\?[^\"]*\"/g,'javascript:pg(0)');
   $('#books').html(r); } ) 
   lp = par;
 }
}

function pg(p) {page=p; RB(1);}
function tp() {}
$(document).ready(RB); 
</script>
<br><h3>Фильтр</h3><table><tr><th>Файл<th>Название<th>Фамилия<th>Имя<th>Жанр<th>Цикл
<tr><td><input id=f onKeyUp=RB()><td><input id=t onKeyUp=RB()><td><input id=ln onKeyUp=RB()><td><input id=fn onKeyUp=RB()>
<td><input id=g onKeyUp=RB()><td><input id=s onKeyUp=RB()></table><div id=books></div>
\n<br><hr><br><h3>Поиск</h3><form action=booklib><input name=a value=\"$ask\"><input type=submit $ButtonStyle value='Искать'></form>\n";
  } 
  return $r.theme('pager');
}

function p($t) { return addslashes(trim(urldecode ($_GET[$t])));}

function printline($a1) {
  $r = "<a href='/booklib/$a1->Filename'>$a1->Filename ($a1->Size b)</a> - $a1->Title - ";
  if ($a1->Seq) $r .= "$a1->Seq $a1->SeqN";
  $r .= "$a1->A1N1 $a1->A1N2 $a1->A2N1 $a1->A2N2 - <font size=-70%> $a1->G1 $a1->G2 $a1->G3 </font><br>";
  return $r;
}

function tow($s) {
  return iconv ("UTF-8", "windows-1251", $s);
}

