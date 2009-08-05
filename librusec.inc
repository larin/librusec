<?php
Global $rates;
$rates = array ('  ', 'нечитаемо', 'плохо', 'неплохо', 'хорошо', 'отлично!');
module_load_include('inc', 'librusec', 'parser');

class DB {
  private $dbname, $idname;
  function DB($dbname = 'libbook') {
    $this->dbname = $dbname;
    $this->idname = 'BookId';
  }
  function __call($m, $a) {
    return Sel("$m FROM $this->dbname WHERE $this->idname = $a[0]");
  }
}


Global $LoadAvg;
// Относительный уровень нагрузки. до 1 - сервер свободен, 1-2 - загружен, больше 2 - сильно загружен.
function LA() {
  Global $LoadAvg;
  if ($LoadAvg > 0) return $LoadAvg;
  $fh = @fopen("/proc/loadavg", 'r');
  if(!$fh) return 1; // non-linux fix
  $Data = fread($fh, 55);
  fclose($fh);
  list ($la1) = split(' ', $Data);
  $fh = fopen("/www/la.sql", 'r');
  $Data = fread($fh, 55);
  fclose($fh);
  list ($la2) = split(' ', $Data);
  $LoadAvg = max($la1, $la2)/3;
  return $LoadAvg; 
}

function IsNewUser() {
  Global $user; 
  if (!$user->uid) return 1;
  $tm = Sel("UNIX_TIMESTAMP() - created FROM users WHERE uid = $user->uid");
  return $tm < 2*24*60*60;
}

function BPP($q = 0) { // books por page
  Global $sa;
  if ($q || $_GET['sa']) $sa = 1;
  $r = $sa ? variable_get('librusec_PPa', libPPa) : variable_get('librusec_PP', libPP);
  if ($r < 10) return 50;
  return $r;
}

function SuperUser() {
  Global $user; 
  return $user->uid == 1;
}

function libcanedit($b = 0) {
  Global $user;
  if (!$user->uid) return 0;
  if ($user->roles[4]) return 0;
  if (variable_get('librusec_Mirror', '0')) {
    print "Добавление и изменение книг невозможно в режиме зеркала. Переключите режим в <a href=/admin/settings/librusec>настройках библиотеки</a>";
    exit;  
  }  
  if (user_access('библиотекарь')) return 1;
  if (IsNewUser()) return 0;
  if (!user_access('исправлять книги')) return 0;
  if (!$b) return 1;
  if (bookquality($b) >= 4.5) return 0;
  if (Sel("block FROM libblocked WHERE BookId = $b")) return 0;
//не библиотекарь хочет редактировать не заблокированную книгу
  $n = Sel ("count(*) FROM libactions WHERE UserName = '$user->name' AND Time > SUBDATE(NOW(), 1)");
  if ($n >= 500) return 0;
  return 1;   
}

Global $ButtonStyle, $ShowEN, $user;
$ShowEN = $user->uid ? 1 : variable_get('librusec_ShowEN', '');
$ButtonStyle = "class=button";
$debug = 0;

// что это за хрень на польском? ;)
# Z carskiej imperii
# Podania i legendy polskie, ruskie i litewskie

// функция, осуществляющая романизацию строки.
// большая просьба отрыть где-нибудь нормальный код, учитывающий все символы.
function translitcyr($cyr_str) {
$transtbl = array(
// кириллица
   "А"=>"A","Б"=>"B","В"=>"V","Г"=>"G",
   "Д"=>"D","Е"=>"E","Ё"=>"Yo","Ж"=>"Zh","З"=>"Z","И"=>"I",
   "Й"=>"Y","К"=>"K","Л"=>"L","М"=>"M","Н"=>"N",
   "О"=>"O","П"=>"P","Р"=>"R","С"=>"S","Т"=>"T",
   "У"=>"U","Ф"=>"F","Х"=>"H","Ц"=>"Ts","Ч"=>"Ch",
   "Ш"=>"Sh","Щ"=>"Sch","Ъ"=>"","Ы"=>"Yi","Ь"=>"",
   "Э"=>"E","Ю"=>"Yu","Я"=>"Ya","а"=>"a","б"=>"b",
   "в"=>"v","г"=>"g","д"=>"d","е"=>"e","ё"=>"yo","ж"=>"zh",
   "з"=>"z","и"=>"i","й"=>"y","к"=>"k","л"=>"l",
   "м"=>"m","н"=>"n","о"=>"o","п"=>"p","р"=>"r",
   "с"=>"s","т"=>"t","у"=>"u","ф"=>"f","х"=>"h",
   "ц"=>"ts","ч"=>"ch","ш"=>"sh","щ"=>"sch","ъ"=>"",
   "ы"=>"yi","ь"=>"","э"=>"e","ю"=>"yu","я"=>"ya",
// дополнительные кириллические символы 
   "Ґ"=>"G","ґ"=>"g","Є"=>"Ye","є"=>"ye","Ї"=>"Yi","ї"=>"yi","І"=>"I","і"=>"i",'ў'=>'w','Ў'=>'W',
   'Џ'=>'Dzh','џ'=>'dzh','Ћ'=>'Ch','ћ'=>'ch','Ј'=>'y','ј'=>'y',
   

// остальные 
   "Ą" => "On","ą" => "on","Ć" => "Ch","ć" => "ch","Ę" => "Eu","ę" => "eu","Ł" => "W","ł" => "w", // pl
   "Ń" => "N","ń" => "n","Ó" => "U","ó" => "u","Ś" => "Sch","ś" => "sch","Ź" => "Zh","ź" => "zh","Ż" => "Zh","ż" => "zh", // pl
   // cz
   "ĉ" => "c","ĵ" => "j","Ĵ" => "J","ŭ" => "u","ĥ" => "h","Ŭ" => "U","Ĥ" => "H",

// дополнительные символы
   " "=>'_', '"'=>'', "'"=>'',":"=>'', "("=>'_',")"=>'_',"["=>'_',"]"=>'_',"{"=>'_',"}"=>'_',
   '…' => '.', '!'=>'','“'=>'','”'=>'', '`'=>'', '—'=>'-', '–'=>'-',"№"=>"N",'$' => 'S',
   '?'=>'',','=>'','\''=>'','«'=>'','»'=>'', '&'=>'_', '#' => 'N');
   for ($i = 1; $i < 32; $i++) $transtbl[chr($i)] = '';
   return str_replace('__','_', strtr($cyr_str, $transtbl));
}

function dbq($st) {
  $args = func_get_args();
  array_shift($args);
  if (isset($args[0]) and is_array($args[0])) $args = $args[0];
  $sth = db_query($st, $args);
  return $sth;
}

function SELECT($st) {
  $args = func_get_args();
  array_shift($args);
  if (isset($args[0]) and is_array($args[0])) $args = $args[0];
  $sth = db_query("SELECT $st", $args);
  return $sth;
}

function Sel($st) { 
  $args = func_get_args();
  array_shift($args);
  return db_result(SELECT($st, $args));
}

function dbf ($s) { 
  return db_fetch_object($s); 
}

function pgq($query, $t) {
  Global $pager_page_array, $pager_total, $pager_total_items;
  $limit = max(1, BPP());
  $element = 0;
  $page = isset($_GET['page']) ? $_GET['page'] : '';
  $args = array(NULL, 0, $PP, $query);
  $pager_page_array = explode(',', $page);
  $pager_total_items[$element] = $t;
  $pager_total[$element] = ceil($pager_total_items[$element] / $limit);
  $pager_page_array[$element] = max(0, min((int)$pager_page_array[$element], ((int)$pager_total[$element]) - 1));
  return db_query_range($query, $args, $pager_page_array[$element] * $limit, $limit);
}

function S($st) {
  $args = func_get_args();
  array_shift($args);
  return db_fetch_object(db_query("SELECT $st", $args));
}

function Insert ($table, $columns, $values) { 
  $args = func_get_args();
  array_shift($args);array_shift($args);array_shift($args);
  return db_query("INSERT DELAYED INTO $table ($columns) values ($values)", $args);
}

function Update ($table, $columns, $cond)   { 
  $args = func_get_args();
  array_shift($args);array_shift($args);array_shift($args);
  return db_query("UPDATE $table SET $columns WHERE $cond", $args);
}

function Delete($st) {
  $args = func_get_args();
  array_shift($args);
  return db_query("DELETE FROM $st", $args);
}

function LogAction ($sql, $desc, $undo, $BookId = 0) {
  if (!libcanedit($BookId)) return;
  if ($BookId) libsettag("b$BookId");
  libsettag(b);
  libsettag(polka);
  Global $user;
  if ($sql) db_query ($sql);
  $sql = addslashes ($sql); 
  $undo = addslashes ($undo);
  Insert ('libactions', 'UserName, ActionSQL, ActionDesc, ActionUndo, BookId', "'%s', '%s', '%s', '%s', $BookId", $user->name, $sql, $desc, $undo);
}

function dmemcache_add($key, $val) {
  if (function_exists(dmemcache_object)) 
    if ($mc = dmemcache_object(librusec)) 
      return $mc->add(dmemcache_key($key, librusec), $val, 0, 0);
}

//libCacheGet
function libcg($cid, $time = 0) {
//  return apc_fetch($cid);
  if ($W = cache_get($cid, librusec)) {
    if ($tag = $W->tag) {
      if (dmemcache_get($tag, librusec) == $W->val) {// версия объекта не изменилась
        apc_inc('Memcache good tag');
        return $W->data;
      } else { // устарелая версия в кэше, надо переделать
        apc_inc('Memcache bad tag');
        return '';
      }
    } elseif ($time == 0 || time() - $W->created < $time || LA() > 1) {
      return $W->data;
    }  
  };
  return '';
}

//libCacheSet
function libcs($cid, $data, $tag = NULL) {
  if (!$cid) return $data;
  if (function_exists(dmemcache_get) && isset($tag)) {
    dmemcache_add($tag, time());
    $val = dmemcache_get($tag, librusec);
    $cache = new stdClass;
    $cache->tag = $tag;
    $cache->val = $val;
    $cache->data = $data;
    dmemcache_set($cid, $cache, CACHE_PERMANENT, librusec); 
  } else {
    cache_set($cid, $data, librusec, CACHE_PERMANENT); 
  }
  return $data;
}

function libsettag($tag) { // увеличиваем значение тэга, инвалидируя этим все зависящие от него кэши.
  if (function_exists(dmemcache_add))
    dmemcache_set($tag, time(), CACHE_PERMANENT, librusec);
}

//libCacheDelete
function libcd($cid) {
//    apc_delete($cid);
  cache_clear_all($cid, librusec);
}

function apc_inc($cid) {
  if(function_exists(apc_store)){
    apc_store($cid, 1+apc_fetch($cid));
  }
}

function GetAvtorId ($n1, $n2, $n3) {
  return Sel("AvtorId FROM libavtorname WHERE FirstName='%s' AND MiddleName='%s' AND LastName='%s'", $n1, $n2, $n3);
}

function AddAvtorId ($n1, $n2, $n3, $n4='')  {
  $n1 = preg_replace("/['\.]+/",'',trim ($n1));
  $n2 = preg_replace("/['\.]+/",'',trim ($n2));
  $n3 = preg_replace("/['\.]+/",'',trim ($n3));
  $a = GetAvtorId ($n1, $n2, $n3);
  if ($a) {
    $a1 = Sel("GoodId FROM libavtoraliase WHERE BadId = $a");
    return $a1?$a1:$a;
  }
  if (!$n3) return '';
  Insert ('libavtorname', 'FirstName, MiddleName, LastName, NickName', "'%s','%s','%s','%s'", $n1, $n2, $n3, $n4);
  return GetAvtorId($n1, $n2, $n3);
}

function AddSeqId ($s) {
  $s = trim($s);
  if (!$s) return 0;
  $id = Sel("SeqId FROM libseqname WHERE SeqName = '%s'", $s);
  if ($id) return $id;
  Insert ('libseqname','SeqName',"'%s'", $s);
  return Sel("SeqId FROM libseqname WHERE SeqName = '%s'", $s);
}

function RemoveBook($b) {
  Delete('libbook WHERE BookId=%d', $b);
  Delete('libgenre WHERE BookId=%d', $b);
  Delete('libavtor WHERE BookId=%d', $b);
  Delete('liblog WHERE BookId=%d', $b);
  Delete('libreaded WHERE BookId=%d', $b);
  Delete('libstat WHERE BookId=%d', $b);
  Delete('libseq WHERE BookId=%d', $b);
  Delete('libblocked WHERE BookId=%d', $b);
  Delete('libfilename WHERE BookId=%d', $b);
  libsettag("b$b");
}

function DeleteBook ($b) {
  $del = Sel("Deleted FROM libbook WHERE BookId = $b");
  LogAction("UPDATE libbook SET Deleted=(1|Deleted) WHERE BookId=$b", "Delete Book $b", "UPDATE libbook SET Deleted='$del' WHERE BookId=$b", $b);
}

function UnDeleteBook ($b) {
  if (Sel("Deleted FROM libbook WHERE BookId = $b") == 1)
    LogAction("UPDATE libbook SET Deleted='' WHERE BookId=$b", "Undelete Book $b", "UPDATE libbook SET Deleted=1 WHERE BookId=$b", $b);
}

function BookDeleteAvtor($b, $a) {
  if (!$b*$a) return;
  LogAction("DELETE FROM libavtor WHERE BookId=$b AND AvtorId=$a LIMIT 1", "Delete Avtor $a from Book $b", 
            "INSERT INTO libavtor (BookID, AvtorId) VALUES ($b, $a)", $b);
}

function BookAddAvtor($b, $a) {
  if (!Sel("AvtorId FROM libavtor WHERE BookId = $b AND AvtorId = $a"))
    LogAction("INSERT INTO libavtor (BookID, AvtorId) VALUES ($b, $a)", "Add Book $b Avtor $a", "DELETE FROM libavtor WHERE BookId=$b AND AvtorId=$a LIMIT 1", $b);
}

function BookDeleteTranslator($b, $a) {
  LogAction("DELETE FROM libtranslator WHERE BookId=$b AND TranslatorId=$a LIMIT 1", "Delete Translator $a from Book $b", 
            "INSERT INTO libtranslator (BookID, TranslatorId) VALUES ($b, $a)", $b);
}

function BookAddTranslator($b, $a) {
  if (!Sel("TranslatorId FROM libtranslator WHERE BookId = $b AND TranslatorId = $a"))
  LogAction("INSERT INTO libtranslator (BookId, TranslatorId) VALUES ($b, $a)", "Add Book $b Translator $a", 
            "DELETE FROM libtranslator WHERE BookId=$b AND TranslatorId=$a LIMIT 1", $b);
}

function BookSetTitle($b, $t, $t1) {
  $t = trim($t); 
  $t1 = trim($t1); 
  $tt = Sel ("Title FROM libbook WHERE BookId = $b");
  $tt1 = Sel ("Title1 FROM libbook WHERE BookId = $b");
  if (!$t || ($t == $tt && $t1 == $tt1)) return;
  $t = addslashes($t);
  $t1 = addslashes($t1);
  $tt = addslashes($tt);
  $tt1 = addslashes($tt1);
  LogAction ("UPDATE libbook SET Title='$t', Title1 = '$t1' WHERE BookId=$b", "Change Title $b from $tt.$tt1 to $t.$t1", 
             "UPDATE libbook SET Title='$tt', Title1 = '$tt1' WHERE BookId=$b", $b);
}

function BookDeleteSeq($b) {
  $seqid = 1*arg(3);
  if (!$seqid) {
    $sth = SELECT("SeqId FROM libseq WHERE BookId = %d", $b);
    while($seqid = db_result($sth)) BookDeleteSeq($b, $seqid);
  } else {
    $a1 = S("* FROM libseq WHERE BookId = $b AND SeqId = $seqid");
    if (!$a1->SeqId) return;
    LogAction("DELETE FROM libseq WHERE BookId=$b AND SeqId = $seqid", "Delete Seq $seqid from book $b", 
              "INSERT INTO libseq (BookId, SeqId, SeqNumb, Level) VALUES($b, $a1->SeqId, '$a1->SeqNumb', '$a1->Level')", $b);
    libcd('libseq'.$b);          
  }  
}

function BookAddSeq($b, $SeqId, $SeqNumb, $level = 0) {
  if (!libcanedit($b)) return;
  LogAction("INSERT INTO libseq (BookId, SeqId, SeqNumb, Level) VALUES($b, $SeqId, $SeqNumb, $level)", "Add Seq $SeqId To book $b", 
            "DELETE FROM libseq WHERE BookId=$b AND SeqId = $SeqId");
  $sn = Sel ("SeqName FROM libseqname WHERE SeqId=$SeqId", $b);     
  $t = preg_replace("/\($sn\s*-\s*$SeqNumb\)/", '', Sel("Title FROM libbook WHERE BookId = %d", $b));
  BookSetTitle($b, $t, Sel("Title1 FROM libbook WHERE BookId = %d", $b));
  libcd('libseq'.$b);          
}

function BookDelGenre($b, $g) {
  if ($g = Sel ("GenreId FROM libgenrelist WHERE GenreId = '$g' OR GenreCode = '$g'"))  
    if (Sel("Id FROM libgenre WHERE BookId=$b AND GenreId=$g LIMIT 1"))
      LogAction("DELETE FROM libgenre WHERE BookId=$b AND GenreId=$g LIMIT 1", "Del genre $g from book $b", "INSERT INTO libgenre (BookId, GenreId) VALUES ($b, $g)", $b);
//    else BlockUserNah();  
}
      
function BookAddGenre($b, $g) {
  if ($g = Sel ("GenreId FROM libgenrelist WHERE GenreId = '$g' OR GenreCode = '$g'"))  
    LogAction("INSERT INTO libgenre (BookId, GenreId) VALUES ($b, $g)", "Add genre $g to book $b", "DELETE FROM libgenre WHERE BookId=$b AND GenreId=$g LIMIT 1", $b);
}

function AvtorName($a, $t=' ', $LastFirst = 0) {
  static $_avtor_names = array();
  if (!($a >= 1)) return '';
  if ($_avtor_names[$a]) return $_avtor_names[$a];
  $a1 = S("FirstName, MiddleName, LastName, NickName FROM libavtorname WHERE AvtorId = $a");

  if (mb_strlen($a1->FirstName) == 1) $a1->FirstName .= '.';
  if (mb_strlen($a1->MiddleName) == 1) $a1->MiddleName .= '.';

  if ($a1->FirstName) $an = $a1->FirstName.$t;
  if ($a1->MiddleName) $an .= $a1->MiddleName.$t;
  if ($LastFirst) {
    if ($an) $an = $t.$an;
    $an = $a1->LastName.$an;
  } else {
    $an .= $a1->LastName;
  }  
  if ($nick) $an .= "$t($nick)";
  $an = check_plain($an);
  if (!$LastFirst) $_avtor_names[$a] = $an;
  return $an;
}

function libSelectFileType($b = 0) {
  Global $user;  
  if (!$user->uid || !variable_get('librusec_Convert', '')) return '';
  $seltp = "<select id=useropt name=dtp onchange=setuseropt('D')>";
  $tp = GetUserOpt('D');
  if (!$tp) $tp = 'fb2';
  foreach (array('fb2', 'html', 'txt', 'rtf', 'pdf', 'epub', 'lrf') as $fmt)  
    $seltp .= '<option '.($fmt==$tp?' selected':'').">$fmt";
  $seltp .= '</select>';
  return $seltp;
}

function downloadall($part, $n = 2, $t = '') {
  Global $ButtonStyle, $user;  
  if (!user_access('выкачивать помногу') || LA() > 2) $dis = 'disabled';
  if ($part == 'up') {
    if ($n <= 1) return libSelectFileType();
    $r = "<form name=bk action=/mass/download target=_blank><input type=checkbox $dis name=all onClick='UCC(this.checked)'>Выбрать всё &nbsp; &nbsp;";
    if ($t == '' or $t == 'fb2') $r .= " Выкачивать: ".libSelectFileType();
    return $r;
  }  
  if (!$user->uid) 
    return "<a href=/user/register>Зарегистрируйтесь</a> / <a href=/user>залогиньтесь</a> для выкачки нескольких книг одним файлом. ";
  $r .= "<br><input type=submit name=downloadall $ButtonStyle value='Выкачать все выбранное' onClick='return confirmmassdownload()'> ".
        "<input type=submit name=readedall $ButtonStyle value='Отметить все выбранное как прочитанное'>";
  return $r;      
}   

/********************************************************************************************************
* Одна из главных функций. Возвращает информацию о книге $b сформатированнку по указаниям в строке $style
* Используется (почти) везде, где надо вывести ссылку на книгу или список книг.
* Параметры в $style перечисляются через пробел в произвольном порядке
**********************************************************************************************************/
function DoLibRow($b, $style='') {
  apc_inc('Dolibrow All');
//style: checkbox, edit, authors, genre, annotation (if $sa), sequence, nolink, nobreak, rate
  Global $lastSecId, $la, $rd, $user, $ShowEN, $sa, $predgen, $rates;
//  if (!user_access('выкачивать помногу') || LA() > 2) $dis = 'disabled';
  if ($b->BookId) {
    $a1 = $b;
    $b = $a1->BookId;
  } else {  
    $a1 = S("* FROM libbook WHERE BookId = $b");
  }  
  if (!$a1 || !$ShowEN && $a1->Lang != 'ru') return ''; 
  $edit = strstr($style,'edit') && !variable_get('librusec_Mirror', '0');
  
  if ($style != 'rateonly' && !libListBlackGG() && (!$user->uid || !strstr($style,'seltype'))) {
    $cid = 'LibRow'.md5($b.$style.$sa).$ShowEN;
    if ($r = libcg($cid)) 
      apc_inc('Dolibrow Hits');
  }    
/*      if ($user->uid && strstr($style,'rate')) 
        return str_replace("<place for rate>", DoLibRow($b, 'rateonly'), $r);
      else   
        return $r;
*/

  if (strstr($style,'genre')) {
    $g = DoGenreRow($b, $a1->g);
    if (strstr($style,'checkbox')) {
      $g = str_replace("<p class=genre>", "<p class=genre><input type=checkbox $dis onclick='UCCg(this.checked, \"$predgen\")'>", $g);
    }  
    if ($edit && $g) $g = '<tr><td>'.$g;
  }  

  $Seq = libcg($cid1 = 'seq'.$b);
  if (!$Seq) {
    $Seq->BookId = $b;
    $sth = SELECT("* FROM libseq JOIN libseqname USING(SeqId) WHERE BookId = $b AND level < 100 ORDER BY level LIMIT 1");
    while ($ss = dbf($sth)) {
      $Seq->ids[] = $ss;
      $Seq->allids .= $ss->SeqId.' ';
    }  
    libcs($cid1, $Seq, "b$b");
  }  
  if ($Seq->ids[0] == $Seq->ids[1]) $Seq->ids[1] = '';
  if (strstr($style,'sequence')) {
    if ($lastSecId != $Seq->allids) { // сериал поменялся
      if ($lastSecId = $Seq->allids) { // на непустой
         for ($i = 0; $ss = $Seq->ids[$i]; $i++) {
           if (strstr($style,'checkbox')) $sec .= "<input type=checkbox $dis onclick='UCCs(this.checked, $ss->SeqId)'>";
           if (strstr($style,'sequencen') && $ss->SeqNumb) $NNN = " - $ss->SeqNumb";
           $sec .= "<a href=/s/$ss->SeqId><h8>$ss->SeqName</h8></a>$NNN<br>\n" ;
         }  
      } else { // на пустой - если не менялся жанр, добавим пустую строчку
         if (!$g) $sec = '<br>';
      }  
    } else { // сериал не менялся
      if ($Seq->allids) $g = ''; 
    } 
    $g .= $sec;
    $nn = $Seq->ids[0]->SeqNumb;
    $sec = $nn > 0 ? "$nn. " : '';
    $secname = '';
  } else {
    $ss = $Seq->ids[0];
    if ($ss->SeqId > 100) 
      $secname = " (<a href=/s/$ss->SeqId><h8>$ss->SeqName</h8></a>".($ss->SeqNumb>0 ? "-$ss->SeqNumb" : '').") ";
  }

  if (!$r) {  // начали собирать

  if ($edit) $r .= '<tr><td>';

  if ($style != 'rateonly') {
    $q = bookquality($b);
    if ($q < 1) $r .= "<img src=/img/znak.gif border=0>"; //не оценённый
    elseif ($q > 4.5) $r .= "<img src=/img/znak5.gif border=0 alt='файл высшего качества'>";
    elseif ($q < 2) $r .= "<img src=/img/znak1.gif border=0 alt='файл недостоин'>";
    else $r .= "<img src=/img/znak.gif border=0>"; 
  }

  $dwncnt = $a1->N;
  if ($user->uid && strstr($style,'rate')) {
    $rate = libGetRate($b);
    $rt = "<select onchange=setrate($b) id=rate$b>";
    for ($i = 0; $i <= 5; $i++) {
      $selected = $i == $rate ? 'selected' : '';
      $rt .= "<option $selected value=$i>$rates[$i]</option>";
    }  
    $rt .= "</select>";
    if ($style == 'rateonly') return $rt;
    $r .= "<place for rate>";
  }
  if ($style == 'rateonly') return '';
  $ast = ''; // список авторов
  if (strstr($style,'authors')) {
    $sth = SELECT ("AvtorId FROM libavtor WHERE BookId = $b LIMIT 5");
    while ($a2 = dbf($sth)) {
      if (!$a) $a = $a2->AvtorId;
      $ast .= ' - '.avl($a2->AvtorId);
    }  
  } elseif (strstr($style,'author')) {
    $sth = SELECT("AvtorId, LastName FROM libavtorname JOIN libavtor USING (AvtorId) WHERE BookId = $b LIMIT 1");
    $a2 = dbf($sth);
    $ast = " - <a href=/a/$a2->AvtorId>$a2->LastName</a>";
    if (!$a) $a = $a2->AvtorId;
  }
  $tp = $a1->FileType;

  if (strstr($style,'checkbox')) {
    if ($a1->Deleted != 7) 
      $r .= "<input type=checkbox $dis id='$predgen-".trim($Seq->allids)."' name=$b>";
    elseif (!$edit) $r .= '&nbsp; &nbsp; &nbsp; ';
  }  

  $size = (integer)($a1->FileSize/1024).'K';  
  
  $title = $a1->Title;
  if (!strstr($style,'nolink')) $title .= "</a>";
  if ($a1->Title1) $title .= " [$a1->Title1]";
  if (!strstr($style,'notrans')) {
    $sth = SELECT("TranslatorId FROM libtranslator WHERE BookId = $b LIMIT 2");
    while ($at1 = dbf($sth)) {
      if ($transid) $trans2id = $at1->TranslatorId;
      else $transid = $at1->TranslatorId;
    }
    if ($transid) {
      $title .= " (пер. ".avl($transid);
      if ($trans2id) $title .= ", ...)";
      else $title .= ")";
    }  
  }  
  if (strstr($style,'nolink')) { // без ссылки, используется на страничке книги
    $r .= "$title <span style=size>$size</span> ";
    if ($dwncnt) $r .= "(книга прочитана $dwncnt раз)";    
  } else { // c cсылкой
    if ($dwncnt) $size .= " ($dwncnt)";
    $r .= " - $sec <a href=/b/$b>$title $secname <span style=size>$size</span>";      
  }

  if ($a1->Deleted != 7) {
    if ($tp == 'fb2') {
      if (!strstr($style,'noread')) 
        $r .= " &nbsp; <a href=/b/$b/read>(читать)</a> &nbsp; <a href=/b/$b/download>(скачать)</a> ";
      else   
        $r .= " <a href=/b/$b/download>(скачать)</a> ";
      if (strstr($style,'seltype')) $r .= ' '.libSelectFileType($b);     
    } else {
      $r .= " <a href=/b/$b/download>(скачать $tp)</a> ";
    }  
    $corr = " <a href=/b/$b/edit>(исправить)</a> ";
    if ($edit) {
      $r.=  "$ast<td>".delbooklink($b, $title)."<td>$corr<td>";
    } else {
      static $divn;
      if ($a1->FileSize > 55553 && !Sel("BookId FROM `libtopadvert` WHERE BookId = $b")) {
        if (!$a) $a = $a1->a;
        if (!$a) $a = Sel("AvtorId FROM libavtor WHERE BookId = $b LIMIT 1");
        $r .= "
<textarea id=topadvert_open_div_$b disabled=true style='display: none'>
title: Купить <b>$a1->Title</b>:
feed_id: 2942
pattern_id: 317
format: 2
book_author: ".AvtorName($a)."
book_name: $a1->Title
</textarea>
&nbsp;
<a href='' onmouseover=\"topadvert_open_div('topadvert_open_div_$b', event);\" onmouseout=\"topadvert_close_div('topadvert_open_div_$b')\" onclick=\"return false;\">(купить)</a>
";
      }
      $r .= $ast;
    }
  }
  if ($a1->Deleted == 7) $r .= " (книга удалена из библиотеки)";
  if ($sa && strstr($style,'annotation')) {
    $ann = ReadAnnotation($b, $nid);
    if ($ann) $r .= "<p>$ann</p>";
  }  
    if (!$ann && !$edit && !strstr($style, 'nobreak')) $r .= '<br>';
    if ($cid) {
      apc_inc('Dolibrow Added');
      libcs($cid, $r, "b$b");
    }  
  }
  return $g.str_replace("<place for rate>", $rt, $r)."\n";
}

function delbooklink ($b, $t) {
  $t = preg_replace ("/'/",'',$t);
  $t = preg_replace ('/"/','',$t);
  return " (<a href=\"javascript:cnf('Вы уверены в необходимости удаления из библиотеки книги $t?', '/b/$b/delete')\">удалить</a>) ";
}

function DoGenreRow($b, $g = '') {
  Global $predgen, $user;
  if (!$g) {
    $cid = 'GR'.$b;
    if (!$g = libcg($cid)) {
      $g = Sel("GROUP_CONCAT(CAST(GenreId AS CHAR) ORDER BY GenreId) FROM libgenre WHERE BookId = $b"); 
      libcs($cid, $g, "b$b");
    } 
  }  
  if ($predgen == $g) return '';
  $predgen = $g;
  $res = '<p class=genre>';
  foreach (split (',' , $g) as $gn) {
    $a1 = S("GenreDesc, GenreCode FROM libgenrelist WHERE GenreId = '$gn'");
    $res .= "<a name=$a1->GenreCode><a class=genre href=/g/$a1->GenreCode>$a1->GenreDesc</a> &nbsp; ";
  }
  return "$res</p>\n";
} 

function avl($a, $t=' ', $o='') {  
  if ($o == 'cnt') $cnt = '('.Sel("SQL_CACHE count(*) FROM libbook JOIN libavtor USING(BookId) WHERE AvtorId = $a AND NOT Deleted&1").')'; 
  if ($t == '&nbsp;') {
    $an = str_replace(' ', $t, AvtorName($a, ' '));
  } else {
    $an = AvtorName($a, $t);
  }
  return "<a href=/a/$a>$an</a>$cnt"; 
}

function bl($b) {  
  $tit = S("Title, Title1 FROM libbook WHERE BookId = $b"); 
  $t = $tit->Title;
  if ($tit->Title1) $t .= " [$tit->Title1]";
  return "<a href=/b/$b>$t</a>"; 
}

function avl1($a) {  // Фамилия, имя, отчество.
  $an = AvtorName($a, ' ', 1);
  return "<a href=/a/$a>$an</a> "; 
}

//формочки разнообразные

function SelectSequence($proc, $skip='', $se='') {
  return "<p>
  <form method=post action=/tools/select/sequence>
  <label>Название сериала</label> 
  <input size=30 name=seq value='$se'>
  <label>Номер серии</label>
  <input size=3 name=seqn>
  <input type=hidden name=proc value='$proc'>
  <input type=hidden name=skip value='$skip'>
  <input type=submit></form>
";
}

function SelectSequenceSubmit($proc, $skip) {
  $se = trim(addslashes($_POST['srcavt']));
  $seqn = 1*$_POST['seqn'];
  $s1 = SELECT ("SeqId, SeqName FROM libseqname WHERE SeqId > 0 AND (SeqId = '$se' OR SeqName LIKE '%$se%') $skip LIMIT 50");
  for ($n = 0; $a1 = dbf($s1); $n++) {
    $SeqId = $a1->SeqId;
    $r .= "<p><a href=/$proc/$SeqId/$seqn>$seqn.$a1->SeqName</a>";
  }  
  if ($n == 0) {
     preg_match('|/(\d+)/$|',$proc,$m);$b=$m[1];
     return "Сериал '$se' не найден. <a href='/b/$b/createseq/$seqn/$se'>(создать)</a>"; 
  } 
  if ($n > 44) return " Найдено слишком много сериалов. Переспросите";
  if ($n > 1) return " Найдено несколько сериалов. Выбирайте и ткните в нужного".$r;
  return lgo ("$proc/$SeqId/$seqn");
}

function SelectGenre($proc, $skip='', $ge='') {
  return "<p>
  <form method=post action=/tools/select/genre>
  <label>Название жанра</label> 
  <input size=30 name=srcgenre value='$ge'>
  <input type=hidden name=proc value='$proc'>
  <input type=hidden name=skip value='$skip'>
  <input type=submit></form>
";
}

function SelectGenreSubmit($proc, $skip='') {
  set_title("Выбор жанра");
  if (!$ge = trim(addslashes($_POST['srcgenre']))) 
    return SelectGenre($proc, $skip).'111';
  $s1 = SELECT ("* FROM libgenrelist WHERE (GenreId = '$ge' OR GenreCode = '$ge' OR GenreDesc LIKE '%$ge%') $skip");
  for ($n = 0; $a1 = dbf($s1); $n++) {
    $gid = $a1->GenreId;
    $r .= "<p><a href=/$proc/$gid>$a1->GenreCode $a1->GenreDesc</a>";
  }  
  if ($n > 1) return "<p>Найдено несколько жаров. Выбирайте и ткните в нужного".$r;
  if ($n == 1) return lgo("$proc/$gid"); 
  preg_match("'/(\d+)/$'", $proc, $mm); $b=$mm[1];
  return "<p>Жанр '$ge' не найден. ";//"<a href=\"/tools/creategenre/$ge/$b\">(добавить жанр)</a>"; 
}

function SelectCreateAvtor($proc, $skip='', $av='') {
  return SelectAvtor ($proc, $skip, $av).
"<br><form action=/tools/createavtor><h3>Добавить отсутствующего автора</h3>
<p>Фамилия: <input name=LastName> 
 Имя: <input name=FirstName> 
 Отчество/второе имя: <input name=MiddleName> 
 Прозвище: <input name=NickName> 
 <input type=hidden name=proclink value='$proc'>
 <input type=submit value='Добавить автора'></form>
";
}

function SelectAvtor ($proc, $skip='', $av='') {
  set_title("Выбор автора");
  return "<p><form method=post action=/tools/select/avtor>
  <label>Фамилия или ID автора</label> 
  <input size=30 name=srcavt value='$av'>
  <input type=hidden name=proc value='$proc'>
  <input type=hidden name=skip value='$skip'>
  <input type=submit></form>
";
}

function SelectAvtorSubmit($proc, $skip) {
  set_title("Выбор автора");
  $av = trim(addslashes($_POST['srcavt']));
  if ($av > 0 && is_numeric($av)) 
   $s1 = SELECT ("AvtorId, CONCAT_WS(' ', FirstName, MiddleName, LastName) as AvtorName FROM libavtorname WHERE (AvtorId = $av) ".$skip);
 else  
   $s1 = SELECT ("AvtorId, CONCAT_WS(' ', FirstName, MiddleName, LastName) as AvtorName FROM libavtorname 
 WHERE (CONCAT_WS(' ', FirstName, MiddleName, LastName) LIKE '%$av%' OR CONCAT_WS(' ', LastName, FirstName, MiddleName) LIKE '%$av%') ".$skip);

  for ($n = 0; $a1 = dbf($s1); $n++) {
    $a = $a1->AvtorId;
    if (strstr($proc, '//')) $p = str_replace('//',"/$a/",$proc);
    else $p = "$proc/$a";
    if ($p[0] != '/') $p = '/'.$p;
    $r .= "<p><a href=$p>$a1->AvtorName</a>";
    if ($aa = Sel ("GoodId FROM libavtoraliase WHERE BadId = $a")) $r .= " (синоним для ". AvtorName($aa).")";
    if ($n > 99) return " Найдено слишком много авторов. Переспросите. ";
  }
  if ($n == 0) return "Автор '$av' не найден. Попробуйте уточнить";
  if ($n == 1) return lgo($p);
  return " Найдено несколько авторов. Выбирайте и ткните в нужного. <br>".$r;
}

function SelectCreateTranslator($proc, $skip='', $av='') {
  set_title("Выбор автора");
  return "<p><form method=post action=/tools/select/avtor>
  <label>Фамилия или ID автора</label> 
  <input size=30 name=srcavt value='$av'>
  <input type=hidden name=proc value='$proc'>
  <input type=hidden name=skip value='$skip'>
  <input type=submit></form>
<br><form action=/tools/createavtor><h3>Добавить отсутствующего переводчика</h3>
<p>Фамилия: <input name=LastName> 
 Имя: <input name=FirstName> 
 Отчество/второе имя: <input name=MiddleName> 
 Прозвище: <input name=NickName> 
 <input type=hidden name=proclink value='$proc'>
 <input type=submit value='Добавить автора'></form>
";
}

function SelectTranslatorSubmit($proc, $skip) {
  return SelectAvtorSubmit($proc, $skip); 
}

function SelectBook($proc, $skip, $bo='') {
  return "<p><form method=post action=/tools/select/book>
  <label>Название или ID книги:</label>
  <input size=30 name=srcbook value='$bo'>
  <input type=hidden name=proc value='$proc'>
  <input type=hidden name=skip value='$skip'>
  <input type=submit></form>";
}

function SelectBookSubmit($proc, $skip) {
  set_title("Поиск книги");
  $bo = trim(addslashes($_POST['srcbook']));
  $n = Sel ("1 FROM libbook WHERE (BookId = '$bo' OR Title = '$bo') $skip LIMIT 1");
  if ($n) $s1 = SELECT ("BookId, Title, Deleted FROM libbook WHERE (BookId = '$bo' OR Title = '$bo') $skip LIMIT 50");
  else $s1 = SELECT ("BookId, Title, Deleted FROM libbook WHERE Title LIKE '%$bo%' $skip LIMIT 50");
  for ($n = 0;$a1 = dbf($s1); $n++) {
    $b = $a1->BookId;
    $av = AvtorName(Sel("AvtorId FROM libavtor WHERE BookId = $b"));
    $r .=  "<p><a href=/$proc/$b>$a1->Title - $av</a>";
    if ($a1->Deleted&1) {$r .= " (удалена) <a href=/b/$b/undel>(восстановить)</a>";}
  }
  if (!$n) return "Книга '$bo' не найдена. Попробуйте уточнить".SelectBook($proc, $skip, $bo);
  if ($n > 44) 
    return $r."<p>Найдено слишком много книг. Переспросите".SelectBook($proc, $skip);
  if ($n > 1) return "Найдено несколько книг. Выбирайте и ткните в нужную".$r;
  return "Книга найдена. Ткните в неё, если если это то, что нужно".$r;
}

function libAddImagesToNode($nid) {
  if (!$nid) return '';
  $ann = Sel("body FROM node_revisions WHERE nid = $nid ORDER BY vid DESC");
  $vid = Sel("MAX(vid) FROM node_revisions WHERE nid = $nid");
  if (!$vid) return '';
  $sth = SELECT("filepath FROM `upload` JOIN files USING(fid) WHERE `vid` = $vid AND filemime LIKE 'image%'");
  while ($f=db_result($sth))  
    if (!strstr($ann, $f)) {
      if ($img) $img .= " &nbsp ";
        $img .= '<img src="/'.$f.'" align=left style="border:5px solid #ededed; margin: 12px;">';
//    $ann = str_replace(array("[img]http://lib.rus.ec/".$f."[/img]", "[img]/".$f."[/img]",  "[img align=left]".$f."[/img]",  "[img=right]http://lib.rus.ec/".$f."[/img]"), '', $ann);
    }
  return $img.check_markup($ann);
}

function ReadAnnotation($b, &$nid='') {
  if (!Sel("BookId FROM libcache WHERE BookId = $b")) {
    unset ($GLOBALS['PD']);
    libRead($b);
  }
  $nid = Sel("nid FROM libbnode WHERE BookId = $b");
  if (!$nid) $nid = Sel("nid FROM node force KEY(node_title_type) WHERE title = $b AND type = 'bdesc'");
  return libAddImagesToNode($nid);
}

function ReadAvtorPage($a, &$nid) {
  $nid = Sel("nid FROM libanode WHERE AvtorId = $a");
  if ($nid) $nid = Sel("nid FROM node_revisions WHERE nid = $nid"); // описание автора не удалено?
  if (!$nid) $nid = Sel("nid FROM node WHERE node.title = $a AND type = 'adesc'");
  return libAddImagesToNode($nid);
}

function ptm($t, $SkipNonUniq = 1) {
  static $_lptm;
  preg_match ('/(\d\d\d\d)-(\d\d)-(\d\d)/',$t,$m);
  $t = "$m[3].$m[2].$m[1]";
  if ($SkipNonUniq && $_lptm == $t) return '';
  return $_lptm = $t;
}

function MakeFileName($b) {
  $a1 = S("* FROM libbook LEFT JOIN libseq USING(BookId) WHERE BookId = $b LIMIT 1");
  $av = Sel("LastName FROM libavtor JOIN libavtorname USING(AvtorId) WHERE BookId = $b LIMIT 1");
  if (strstr($av, 'Неизвестен')) $av = '';
  if ($av) $av .= '_';
  if ($a1->SeqNumb > 0) $av .= Sel("SeqName FROM libseqname WHERE SeqId = '$a1->SeqId'").'_'.$a1->SeqNumb.'_';
  $av = str_replace('/', '_', $av.$a1->Title);
  $outname = translitcyr($av).".$b";
  return $outname;
}

function Undellink($b) {
  if (Sel("Deleted&1 FROM libbook WHERE BookId = $b") != 1) return '';
  if ($newb = Sel("GoodId FROM libjoinedbooks WHERE BadId = $b")) 
    return "<h4>Книга $b заменена на <a href=/b/$newb>исправленную</a> <a href=/b/$b/delalias/$newb>(удалить связь)</a></h4>\n";
  return "<h4>Книга $b удалена из библиотеки. <a href=/b/$b/undel>(восстановить)</a> <a href=/b/$b/join>(указать правильную)</a></h4>\n";
}  

function GetFileName($b) {
  if (Sel ("FileType FROM libbook WHERE BookId = $b") !== 'fb2') 
    return "b.usr/".Sel("FileName FROM libfilename WHERE BookId = $b");
  return "b/$b.fb2";
}

function set_title($t) {
  drupal_set_title($t);
}

function lgo($page) {
  $page = preg_replace('/^\//', '', $page);
  $page = preg_replace('/\/\//', '/', $page);
  drupal_goto($page);
  return "<a href=/$page>$page</a><script LANGUAGE=JavaScript>window.location=\"/$page\"</script>";
}

function GetVer($b) {
  Global $Version; $Version = '';
  if ($v = Sel ("Ver FROM libbook WHERE BookId = $b")) return $v;
  if (Sel("FileType FROM libbook WHERE BookId = $b") != 'fb2') return 0;
  if ($err = parsefb2($f, 0, 'head')) return 0;
  Global $PD;
  if ($v = $PD->head['FICTIONBOOK/DESCRIPTION/DOCUMENT-INFO/VERSION']) 
    db_query("Update libbook SET Ver = '%s' WHERE BookId = %d", $v, $b);
  return $v;
}

function GetUserOpt($o) {
  Global $user;
  if (!$u=$user->uid) return '';
  return Sel ("Value FROM libuseropt WHERE User = $u AND Opt = '$o'");
}

function SetUserOpt($o, $v) {
  Global $user;
  if (!$u=$user->uid) return;
  if ($val = Sel ("Value FROM libuseropt WHERE User = $u AND Opt = '$o'")) {
    if ($val != $v) Update (libuseropt, "Value = '$v'","User = $u AND Opt = '$o'");
  } else {
    Insert ('libuseropt', "User, Opt, Value", "$u, '$o', '$v'");
  }  
}

function AnnChkbox($onClick = "submit()") {
  Global $sa, $rd, $user;
  if ($_GET['sa']) {
    $sa = 'checked'; 
    SetUserOpt('A', '+');
  } elseif ($_GET['sa1']) {	
    SetUserOpt('A', '-');
  } else {//первый раз показывает - чекбокса нету и не было
    if (GetUserOpt('A') == '+')
      $sa = 'checked'; 
  }
  if ($_GET['rd']) {
    $rd = 'checked'; 
    SetUserOpt('R', '+');
  } elseif ($_GET['rd1']) {
    SetUserOpt('R', '-');
  } else {//первый раз показывает - чекбокса нету и не было
    if (GetUserOpt('R') == '+')
      $rd = 'checked'; 
  }
  if ($user->uid) {
    if ($sa) $r .= '<input type=hidden name=sa1 value=1>';
    if ($rd) $r .= '<input type=hidden name=rd1 value=1>';
    $r .= "<input type=checkbox name=rd onClick=$onClick $rd>Скрыть прочитанное";
  }  
  $r .= "<input type=checkbox name=sa onClick=$onClick $sa>Аннотации"; 
  return $r;
}  

/******************************************************************************************
* Система кэширования в файлах с элементами неестественного интеллекта
* При повышенной загрузке отдаёт старьё, при пониженной - перегенерирует при необходимости.
* На время генерации хапает семафор, чтоб свежепротухшая популярная страничка 
* не генерировалась десятком клиентов одновременно
*******************************************************************************************/
function libOpenCacheFile($CacheFile, $tm) {
  Global $user, $rd, $key;
  $CacheIsPossible = $user->uid != 1 && !$rd && file_exists($CacheFile);
  if ($CacheIsPossible && (time() - 1*filemtime($CacheFile) < rand($tm, 1.2*$tm) || LA() > 1)) return 0;
  $key = sem_get(abs(crc32($CacheFile) % 5) + 6, 30);
  if (!sem_acquire($key)) {
    $key = 0;
    return 0;
  }  
  if ($CacheIsbossiple && (time() - 1*filemtime($CacheFile) < $tm || LA() > 1)) return 0;
  return fopen($CacheFile, 'w');
}

function libReadCacheFile($CacheFile) {
  Global $key;
  if ($key) sem_release($key);
  return file_get_contents ($CacheFile);
}    

function bookav ($b) {
  $s = SELECT ("* FROM libavtor JOIN libavtorname USING (AvtorId) WHERE BookId = $b");
  while ($a = dbf($s)) 
    $r .= "<a href=/a/$a->AvtorId>$a->FirstName $a->MiddleName $a->LastName</a> &nbsp;\n";
  return $r;
}

function bookgenres($b) {  
  return Sel("GROUP_CONCAT(GenreDesc) FROM libgenre JOIN libgenrelist USING (GenreID) WHERE BookId = $b");
}

function libRSS($CacheFile, $hours, $name, $link, $join, $where) {
// return "SELECT BookId, Title, CONCAT(DATE_FORMAT(Time, '%a, %e '), LEFT(DATE_FORMAT(Time, '%M'),3), DATE_FORMAT(Time, ' %Y %T')) AS T FROM libbook $join WHERE $where NOT (Deleted&1) AND NOW() - Time > 10000 ORDER BY BookId DESC LIMIT 50";
  $CacheFile = "cache/$CacheFile.rss";
  if ($name) $name = "- $name";
  if ($fh = libOpenCacheFile($CacheFile, 3600*$hours)) {
    $host = preg_replace('/^www./','',$_SERVER['HTTP_HOST']);
    fwrite($fh, '<?xml version="1.0" encoding="utf-8"?>
<rss version="2.0">
<channel>
<title>Новости Либрусека'." $name</title>
<link>http://$host/$link</link>
<description>Последние книги, появившиеся на Либрусеке</description>
");
    $sth = SELECT ("BookId, Title, CONCAT(DATE_FORMAT(Time, '%a, %e '), LEFT(DATE_FORMAT(Time, '%M'),3), DATE_FORMAT(Time, ' %Y %T')) AS T FROM libbook $join WHERE $where NOT (Deleted&1) AND NOW() - Time > 10000 ORDER BY BookId DESC LIMIT 50");
      while ($a1 = dbf($sth)) {
        $b = $a1->BookId;
        $a = AvtorName(Sel("AvtorId FROM libavtor WHERE BookId = $b"));
        $g = Sel ("GenreDesc FROM libgenre JOIN libgenrelist USING(GenreId) WHERE BookId = $b");
        $an = check_plain(ReadAnnotation($b,$nid));
        fwrite($fh, "<item>\n<title>".check_plain("$a1->Title - $a - $g")."</title>\n<link>http://$host/b/$a1->BookId</link>\n<guid>http://$host/b/$a1->BookId</guid>\n<pubDate>$a1->T GMT</pubDate>\n<description>$an</description>\n</item>\n");
      }
    fwrite($fh, "</channel>\n</rss>\n");      
    fclose($fh);
  }
  ob_clean();
  header('Content-Type: application/rss+xml; charset=utf-8'); 
  echo libReadCacheFile($CacheFile);
  exit;
}

function GET($t) { return mysql_real_escape_string(trim(urldecode($_GET[$t])));}
function POST($t) { return mysql_real_escape_string(trim(urldecode($_POST[$t])));}

function genres($sid) {
  $all = (0 == $sid) ? 'selected':'';
  $o = "<option $all value='0'>Все жанры</option>\n";
  $genres = SELECT("GenreId, GenreMeta, GenreDesc FROM libgenrelist ORDER BY GenreMeta, GenreDesc");
  $meta = "";
  while ($g = dbf($genres)) {
    $selected = ($g->GenreId == $sid) ? 'selected':'';
    if($g->GenreMeta != $meta){
      if($meta != '') $o .= "</optgroup>";
      $meta = $g->GenreMeta;
      $o .= "<optgroup label='$meta' value='1'>\n";
    }
    $o .= "<option $selected value='$g->GenreId'>$g->GenreDesc</option>\n";
  }
  if($meta != '') $o .= "</optgroup>";
  $url = url(arg(0).'/'.arg(1), array(query => 'genre='));
  return "<select name='genres' onchange=\"document.location='$url'+this.value;\">".$o."</select>";
}

function BlockUserNah() {
  Global $user;
  if (user_access('библиотекарь')) return;
  if (Sel ("uid FROM users_roles WHERE uid = $user->uid AND rid=4")) return;
  Insert('users_roles', 'uid, rid', "$user->uid, 4");
}

function bookquality($b, $reset = 0) {
  if ($reset) libsettag("b$b");
  $cid = 'q'.$b;
  if ($q = libcg($cid)) 
    if ($q && !$reset)
      return $q > 0 ? $q : 0;
  $q2 = Sel ("ROUND(AVG(q),1) FROM libquality JOIN users_roles USING(uid) WHERE BookId=$b AND rid=3"); // мнение админов
  $q3 = Sel ("ROUND(AVG(q),1) FROM libquality WHERE BookId = $b");// общее мнение  
  $q4 = $q2 ? $q2 : $q3;  
  $q = $q4 > 0 ? $q4 : -1;
  libcs($cid, $q);
  return $q;
}

function ul($u) {
 if ($u >= 1) 
  return "<a href=/user/$u>".Sel("name FROM users WHERE uid = $u")."</a>";
}

function MakeZipFile($b, $dtp) {
  Global $user;
  $FileName = MakeFileName($b);
  $r = "b.$dtp/$FileName.$dtp";
  if ($dtp == 'epub') {
   $zip = $r;
  } else { 
    $zip = "$r.zip";
  }  
//  if (file_exists($zip) && (filemtime($zip) < filemtime("b/$b.fb2") || filemtime($zip) < filemtime("modules/librusec/book.inc") || $user->uid == 1)) unlink ($zip);
  if (file_exists($zip))   
    return $zip;
  $key = sem_get($b%5 + 1, 30);
  if (!sem_acquire($key)) return '';
  if (file_exists($zip) && $user->uid != 1) {
    sem_release($key);
    return $zip;
  }
  if ($dtp == 'fb2' || $dtp == '') {
    if (!libGenerateFB2("b/$b.fb2", $r)) 
      exec ("ln b/$b.fb2 \"$r\"");
    exec ("zip -mj \"$zip\" \"$r\"");
  } else {  
    if ($err = MakeFile($b, "b/$b.fb2", $dtp, $r)) {
      drupal_set_message($err);
      watchdog('librusec', "Ошибка создания файла $zip: $err");
      $zip = '';
    } else { 
      if ($zip != $r) {
        exec ("zip -j -m \"$zip\" \"$r\"");
        if (($dtp == 'txt' || $dtp == 'html') && glob("i/".($b%100)."/$b/*")) exec ("zip -j \"$zip\" i/".($b%100)."/$b/*");
      }  
    }  
  }  
  sem_release($key);
  return $zip;
}  

function libWST($b, $reset = 0) {
  if (!$b) return '';
  $cid = "wst".$b;
  if ($reset) return libcd($cid);
  if ($wst = libcg($cid)) return $wst;
  $bb[$b] = 1;
  for ($bn = 0; $bn < count($bb); ) {
    $bn = count($bb);
    foreach ($bb as $b1 => $i1) {
      if ($i1 == 1) {  
        $bb[$b1] = 2;
        $sth = SELECT("BadId FROM libjoinedbooks JOIN libbook ON (BookId=BadId) WHERE GoodId = $b1 AND (Deleted&1)");
        while ($a2 = dbf($sth)) {
          $b2 = $a2->BadId;
          $bb[$b2] = 1;
          $wst .= " OR BookId = $b2";
        }  
      }  
    }  
  }
  return libcs($cid, "(BookId = $b$wst)", "b$b");
}

function libGetRate($b) {
  static $_libgetrate = array();
  Global $user; $u = $user->uid;
  if (!$u) return '';
  $cid = "librate$u";
  if (!$_libgetrate['user'])
    if ($cache = libcg($cid))
      $_libgetrate = $cache;
  if ($_libgetrate['norates']) return '';
  if ($_libgetrate[$b])
    return $_libgetrate[$b] > 0 ? $_libgetrate[$b] : '';
  $_libgetrate['user'] = $u;
  if (Sel("Rate FROM librate WHERE UserId = $u LIMIT 1")) { // у пользователя есть оценки хоть чего-то
    $r = Sel("Rate FROM librate WHERE UserId = $u AND ".libwst($b)." LIMIT 1");
    if ($r) $_libgetrate[$b] = $r;
    else $_libgetrate[$b] = -1;
  } else { // оценок нет вообще, запомним этот факт чтоб больше не дёргаться
    $_libgetrate['norates'] = 1;
    $r = '';
  }  
  libcs($cid, $_libgetrate, "b$b"); 
  return $r;
}

function LibListGenres($b) {
  Global $user;
  if (!$user->uid) return '';
  $sht = SELECT("* FROM libgenre JOIN libgenrelist USING (GenreId) WHERE BookId = %d", $b);
  while($a1 = dbf($sth)) {
    $r .= "$a1->GenreDesc <a href=/g/$a1->GenreId/hide>(скрыть)</a> ";
  } 
  return $r ? "($r)" : '';
}