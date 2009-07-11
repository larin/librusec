<?php

function on_ajax($cmd){
  //not used yet
  switch($cmd) {
    case 'w':
      return stat_read_weekly(GET('genre'));
      exit;
    case '24':
      return stat_read24(GET('genre'));
      exit;
    case 'b':
      return stat_top(GET('genre'));
      exit;
  }
}

function stat_read24($genre){
  $r .= "<ul>";
  if($genre != 0){
    $join = "JOIN libgenre USING(BookId)";
    $where = "libgenre.GenreId=".mysql_real_escape_string($genre);
  }else{
    $join = "";
    $where = "1";
  }
  
  $sth = SELECT ("SUM(1) as Num, liblog.BookId as BookId FROM `liblog` JOIN libbook USING (BookId) $join
                  WHERE $where AND liblog.Time > SUBDATE(NOW( ), 1)
                  GROUP BY 2 ORDER BY 1 DESC LIMIT 100");
  while ($a1=dbf($sth)) {  
    $av = libAvtorsNames($a1->BookId);
    $r .= "<li>$a1->Num: $av - ".bl($a1->BookId);
  }
  $r .= "</ul>";
  return $r;
}

function stat_read_weekly($genre){
  $r = "<ul>";
  if($genre != 0){
    $join = "JOIN libgenre USING(BookId) WHERE libgenre.GenreId=".mysql_real_escape_string($genre);
  }else{
    $join = "";
  }
  
  $sth = SELECT ("SUM(1) as Num, BookId FROM liblog as l ".$join." GROUP BY 2 ORDER BY 1 DESC LIMIT 200");
    
  while ($a1=dbf($sth)) {
    $b = $a1->BookId;
    $n = $a1->Num + Sel("SUM(1) FROM libreaded WHERE BookId = $b AND Time > SUBDATE(NOW( ), 7)");
    $gb = Sel("GoodId FROM libjoinedbooks WHERE BadId = $b");
    if ($gb) $READED[$gb] += $n;
    else $READED[$b] += $n;
  }
  arsort($READED);  
  foreach($READED as $b=>$n) {
    if ($i++>100) break;
    $av = libAvtorsNames($b);
    $r .= "<li>$n: $av - ".bl($b);
  }
  $r .= "</ul>";
  return $r;
}

function stat_top($genre){
  $r = "<ul>";
  if($genre != 0){
    $q = "SELECT b.N as Num, b.BookId FROM libbook b JOIN libgenre g USING(BookId) WHERE g.GenreId=$genre and NOT Deleted&1 ORDER BY 1 DESC";
    $count = "SELECT sum(1) FROM libbook b JOIN libgenre g USING(BookId) WHERE g.GenreId=$genre and NOT Deleted&1 ORDER BY 1 DESC";
  }else{
    $q = "SELECT b.N as Num, b.BookId FROM libbook as b WHERE NOT Deleted&1 ORDER BY 1 DESC";
    $count = "SELECT count(*) FROM libbook WHERE NOT Deleted&1";
  }
  $sth = pager_query($q, 100, 0, $count);
  while ($a1=dbf($sth)) {  
    $av = libAvtorsNames($a1->BookId);
    $r .= "<li>$a1->Num: $av - ".bl($a1->BookId);
  }
  $r .= "</ul>";
  return $r;
}
?>