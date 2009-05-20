<?
include_once drupal_get_path('module', 'librusec') .'/librusec.inc';

function libLetter($buk) {
  set_title("Авторы, фамилия которых начинается на букву $buk");
  return libList('a', $buk);
}

function p($t) { 
  return addslashes(trim(urldecode($_GET[$t])));
}

function libAJAXlist() {
  Global $sa, $rd;
  $sa = $_GET['sa'];
  $rd = $_GET['rd'];
  $W = 'WHERE NOT (Deleted&1)';
  $tm = $_GET['tm'];//Time

  if ($t = p('t')) {$t=str_replace('*','%',$t); $W .= " AND Title LIKE '$t%'";}
  if ($t = p('ln')) if ($t == 'Прочее') $W .= " AND LastName < 'Z' "; else $W .= " AND LastName LIKE '$t%'";
  if ($t = p('fn')) $W .= " AND FirstName LIKE '$t%'";
  if ($t = p('mn')) $W .= " AND MiddleName LIKE '$t%'";
  if ($t = p('g')) {
    $wst = '';
    foreach (split (',', $t) as $l) {
      if ($l = trim($l)) {
        if ($wst) $wst .= " OR";
        $wst .= " GenreDesc LIKE '%$l%' OR GenreCode LIKE '$l%'";
      }  
    }
    $W .= " AND ($wst)";
  }  
  if ($t = p('e')) {
    $wst = '';
    foreach (split (',', $t) as $l) {
      if ($l = trim($l)) {
        if ($wst) $wst .= " OR";
        $wst .= " FileType = '$l'";
      }  
    }
    $W .= " AND ($wst)";
  }  
  if ($t = p('s1')) $W .= " AND FileSize >= '$t'*1024";
  if ($t = p('s2')) $W .= " AND FileSize <= '$t'*1024";
  if ($t = p('lng')) {
    $wst = '';
    foreach (split (',', $t) as $l) {
      if ($l = trim($l)) {
        if ($wst) $wst .= " OR";
        $wst .= " Lang = '$l'";
      }  
    }
    $W .= " AND ($wst)";
  }
  if ($_GET[st1]) $O = "Title"; 
  if ($_GET[st2]) $O = "Title DESC"; 
  if ($_GET[sln1]) $O = "LastName, FirstName, MiddleName, Title"; 
  if ($_GET[sln2]) $O = "LastName DESC, FirstName, MiddleName, Title"; 
  if ($_GET[sfn1]) $O = "FirstName"; 
  if ($_GET[sfn2]) $O = "FirstName DESC"; 
  if ($_GET[smn1]) $O = "MiddleName"; 
  if ($_GET[smn2]) $O = "MiddleName DESC"; 
  if ($_GET[sg1]) $O = "GenreDesc"; 
  if ($_GET[sg2]) $O = "GenreDesc DESC"; 
  if ($_GET[ss1]) $O = "FileSize"; 
  if ($_GET[ss2]) $O = "FileSize DESC"; 
  if ($_GET[se1]) $O = "FileType"; 
  if ($_GET[se2]) $O = "FileType DESC"; 
  if ($_GET[sd1]) $O = "BookId"; 
  if ($_GET[sd2]) $O = "BookId DESC"; 
   
  if (!$O) $O = ' BookId DESC';

  $se = $_GET['se'];//SeqId
  $l = $_GET['l'];//Lang
  $y = $_GET['y'];//Year

  foreach (array('ln','fn','mn') as $qq)  
    if ($_GET[$qq] || $_GET['s'.$qq.'1'] || $_GET['s'.$qq.'2']) 
      $ja = "JOIN libavtor USING(BookId) JOIN libavtorname USING(AvtorId)";

  if ($_GET['g'] || $_GET['sg1'] || $_GET['sg2']) {
    $jg = "JOIN libgenre USING(BookId) JOIN libgenrelist USING(GenreId)";
    $gb = "GROUP BY 1";
  }
  if ($_GET['ab']=='ab2') {
    $SinBooks = 1; 
    if (!$ja) $ja = "JOIN libavtor USING(BookId)";
    $tot = Sel("COUNT(DISTINCT(libavtor.AvtorId)) FROM libbook $ja $jg $W");
    $S = "SELECT libavtor.AvtorId AS a, COUNT(libbook.BookId) as N FROM libbook $ja $jg $W GROUP BY 1 ORDER BY $O";
  } else { 
    $tot = Sel ("COUNT(DISTINCT(libbook.BookId)) FROM libbook $ja $jg $W");
    $S = "SELECT libbook.BookId AS b FROM libbook $ja $jg $W $gb ORDER BY $O";
  }  
  Global $user; if ($user->uid == 1) print "<br>$S<br>";
  $sth = pgq($S, $tot);
  print "<div align=left>";
  if (!$SinBooks) print downloadall('up'); //AnnChkbox('RB()').

  for ($NN = 0; $a1 = dbf($sth); $NN++) {
    if ($SinBooks) 
      print avl1($a1->a)." ($a1->N)<br>\n"; 
    else 
      print DoLibRow($a1->b, 'authors checkbox genre annotation'); 
  }
  if (!$SinBooks) print downloadall('down')."</form>";
  print "</div><br>";
  if (!$NN) print "Не нашлось ни единой книжки, удовлетворяющей вашим втребованиям. Упростите.";
  $pager = theme('pager');
  $pager = preg_replace('|/makebooklist.page=(\d+)"|','javascript:pg($1)"',$pager);
  $pager = preg_replace('|/makebooklist.page=(\d+)[^"]+"|','javascript:pg($1)"',$pager);
  $pager = preg_replace('|/makebooklist[^"]*|','javascript:pg(0)',$pager);
  print $pager;
}

function libList($type='', $FirstLetter='') {
  if ($FirstLetter && $type == 'b') {
    $FirstTitle = $FirstLetter;
    $FirstLetter = '';
  }
  if ($FirstLetter) $sln1 = ' checked';  
  if ($type == 'a') $ab = 'ab=ab2&';
  elseif ($type == 'b') $ab = 'ab=ab1&';
  else $ab = '';
  $sth = SELECT("DISTINCT Lang FROM libbook WHERE Lang <> ''");
  while ($v = db_result($sth)) $ll .= "$v, ";
  $ll = rtrim($ll, ', ');
  $sth = SELECT("DISTINCT FileType FROM libbook");
  while ($v = db_result($sth)) $tt .= "$v, ";
  $tt = rtrim($tt, ', ');

  $r = "<script>var page='$_GET[page]'";
  $r .= '
var t1 = "<table><tr><td>";
var t2 = "</table>";
var lp = "1";
var ltm = 0;
var lp1 = "";
var type = "";
var cnt=0;
var tt;

function libRB(libnow) {
 cnt++;
  
 var tm = new Date; tm = tm.getTime();
 if (libnow != 1 && tm - ltm < 1555) { setTimeout(libRB, 333); return; } //были недавно, зайдите попозже.
 ltm = tm;
 var par = "";
 if (page) par = "page="+page+"&";
 var a = ["t", "g", "ln", "fn", "mn", "s1", "s2", "e", "lng"];
 for (var i in a) {
   var q = jQuery("#"+a[i]).val();
   if (q) par += a[i] + "=" + encodeURI(q) + "&";
 }  
 jQuery("input[@type=radio][@checked]").each(function(i){par += this.value + "=1&"});
 if (jQuery("#sa[@checked]").val()) par += "sa=1&";
 if (jQuery("#rd[@checked]").val()) par += "rd=1&";
 if (libnow != 1 && lp1 != par) {//прошлый раз просили не это. Еще не определились. Зайдите попозже.
   lp1 = par;
   setTimeout(libRB, 1111);
   return;
 }
 if (lp != par) {
   jQuery("#books").html("<p>грузим...</p>");
   jQuery.get("/makebooklist?"+par+"'.$ab.'", function(r){ jQuery("#books").html(t1+r+t2); } ) 
   lp = par;
 }  
}
function pg(p) {page=p; libRB(1);}
</script> 
<style>th {font-size: 70%;padding:0px;spacing:0px}</style>'."
<ul>
<li>Жанры (<a href=/g>список</a>) можно указать несколько через запятую
<li>Языки: ($ll), можно указать несколько через запятую.
<li>Форматы: ($tt), можно указать несколько через запятую.
<table id=tt>
<tr>";
if ($type != 'a') $r .= "<th nowrap>Название";
$r .= "
<th nowrap>Фамилия
<th nowrap>Имя
<th nowrap>Отчество
<th nowrap>Жанр
<th nowrap>Размер файла
<th nowrap>Дата
<th nowrap>Формат  
<th nowrap>Язык  

<tr>";
if ($type != 'a') $r .= "
<td nowrap><input type=radio name=r value=st1 onchange=libRB()><img src=/img/s1.gif>
<input type=radio name=r value=st2 onchange=libRB()><img src=/img/s2.gif>";
$r .= "
<td nowrap><input type=radio name=r value=sln1 onchange=libRB() $sln1><img src=/img/s1.gif>
<input type=radio name=r value=sln2 onchange=libRB()><img src=/img/s2.gif>  
<td nowrap><input type=radio name=r value=sfn1 onchange=libRB()><img src=/img/s1.gif>
<input type=radio name=r value=sfn2 onchange=libRB()><img src=/img/s2.gif>  
<td nowrap><input type=radio name=r value=smn1 onchange=libRB()><img src=/img/s1.gif>
<input type=radio name=r value=smn2 onchange=libRB()><img src=/img/s2.gif>  
<td nowrap><input type=radio name=r value=sg1 onchange=libRB()><img src=/img/s1.gif>
<input type=radio name=r value=sg2 onchange=libRB()><img src=/img/s2.gif>
<td nowrap><input type=radio name=r value=ss1 onchange=libRB()><img src=/img/s1.gif>
<input type=radio name=r value=ss2 onchange=libRB()><img src=/img/s2.gif>
<td nowrap><input type=radio name=r value=sd1 onchange=libRB()><img src=/img/s1.gif>
<input type=radio name=r value=sd2 onchange=libRB()><img src=/img/s2.gif>

<tr>";
if ($type != 'a') $r .= "
<td><input id=t size=10 onKeyUp=libRB() value='$FirstTitle'>";
$r .= "
<td><input id=ln size=8 onKeyUp=libRB() value='$FirstLetter'>
<td><input id=fn size=8 onKeyUp=libRB()>
<td><input id=mn size=8 onKeyUp=libRB()>
<td><input id=g size=10 onKeyUp=libRB()>
<td nowrap>от <input id=s1 size=3 onKeyUp=libRB()>K до <input id=s2 size=3 onKeyUp=libRB()>K
<td>&nbsp;
<td><input id=e size=2 onKeyUp=libRB()>
<td><input id=lng size=2 onKeyUp=libRB()>
</table>";

  if (!$type) $r .= " <input type=radio name=ab value=ab1 onchange=libRB()>Книги <input type=radio name=ab value=ab2 onchange=libRB()>Авторы\n";
  $r .= "</form>\n";

  $r .= "<div id=books><br></div>";
  $r .= "<script>jQuery(document).ready(libRB)</script>"; 
  return $r;
}