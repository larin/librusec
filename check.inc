<?php
module_load_include('inc', 'librusec');

Global $path, $PP;

function libCheck() {
  switch (arg(1)) { 
    case createtorrent:
      print createtorrent;
      $b = (int)arg(2);
      CreateTorrent($b);
      exit;
    break;
    case torrents: {
      require_once('modules/bittorrent/includes/bt_common.inc');
      foreach(glob('all/*torrent') as $file) {
        print $file;
        if ($torrent = bdecode_file($file)) {
          print " decoded<br>";
//          INSERT(bt_torrents, "nid, fid, info_hash, metadata", "136104, 136, '%s', %b", info_hash($torrent), bencode($torrent));
          INSERT(bt_torrents, "nid, fid, info_hash", "136104, 136, '%s'", info_hash($torrent));
        } 
      }
    } 
    exit;
    case splittit:
      $sth = SELECT("* FROM `libbook` WHERE `Title` LIKE '%[%]%' AND Title1 = ''");
      while ($a1 = dbf($sth)) {
        $b = $a1->BookId;
        $r .= bl($b)."<br>";
        if (preg_match('/(.+)\s*\[(.+)\].*/', $a1->Title, $m)) {
          if ($m[1] && $m[2] &&!$m[3]) {
            UPDATE(libbook, "Title='%s', Title1='%s'", "BookId=%d", $m[1], $m[2], $b);
          }  
        }
      }
      return $r;
    case db:
      return libdb();
    case schema:
      return libCheckSchema(arg(2), arg(3));
    case topadv:
      return checktopadv(arg(2)); 
    case read:
      return libNewRead(arg(2));
    case badnames:
      $sth = select("* from libfilename JOIN libbook USING (BookId) where filename like '%?%'");
      while ($a1 = dbf($sth)) {
        $r .= "<li><a href=/b/$a1->BookId>$a1->Title</a>";
      }
      return $r;
    case check:
    $b = 123456;
    return  'lb'.((int)($b/100));
    print session_name(); exit;
$rubr = array('transport', 'transport', 'sport', 'milym-damam', 'milym-damam', 'milym-damam', 'raznoe', 'raznoe', 'raznoe', 'raznoe', 'raznoe', 'raznoe', 'raznoe', 'raznoe', 'raznoe', 'raznoe', 'raznoe', 'raznoe', 'raznoe', 'raznoe', 'raznoe', 'raznoe', 'raznoe', 'raznoe', 'raznoe', 'raznoe', 'raznoe', 'raznoe', 'detjam', 'avto-i-moto', 'puteshestvija-i-turizm', 'puteshestvija-i-turizm', 'puteshestvija-i-turizm', 'biznes-v-internete', 'biznes-v-internete', 'biznes-v-internete', 'biznes-v-internete', 'smi', 'obrazovanie-i-nauka', 'obrazovanie-i-nauka', 'dosug', 'uslugi', 'banki-i-strahovanie', 'banki-i-strahovanie', 'stroitelstvo-i-remont', 'stroitelstvo-i-remont', 'stroitelstvo-i-remont', 'stroitelstvo-i-remont', 'nedvizhimost', 'internet-magaziny', 'internet-magaziny', 'internet-magaziny', 'internet-magaziny', 'internet-magaziny', 'internet-magaziny', 'internet-magaziny', 'internet-magaziny'); 
$rr = '/'.$rubr[rand(0, count($rubr)-1)].rand(count($rubr)-1);
print $rr;
print rand(0, count($rubr));
 exit;   
    
    case libcg:
      print "<pre>";
      $s = libcg(arg(2));
      print_r($s);
    exit;
    
   case SetActionBookId: return SetActionBookId();
   case translator:
     print '1111';
     return find_keywords($qq);
     $f = fopen("cache/batch.log",'a+');
     fprintf($f, 'starts');
     fclose ($f);
     batch_set(array(
       'title' => t('Прочёсываем'),
       'operations' => array(array('find_keywords', array())),
        'finished' => 'my_finished_callback',
    'init_message' => t('Check Batch starting.'),
    'progress_message' => t('Processed @current out of @total.'),
    'error_message' => t('Batch 1 has encountered an error.'),
         'file' => drupal_get_path('module', 'librusec') . '/check.inc',    
     ));
   batch_process();
   return 'batched';
//       return "$a - $ln $fn $mn<pre>".print_r($PD->head, 1);

break;
   case delnul:
      foreach (array(109324) as $b) {
        $r .= $b.' ';
        RemoveBook($b);
      }  
      return $r;
      $sth = SELECT ("BookId FROM libbook WHERE FileSize = 0");
      while ($a1=dbf($sth)) 
        RemoveBook($a1->BookId);
break;  
    case SERVER: 
      print "<pre>"; print_r ($_SERVER); print "</pre>";
      phpinfo(); 
    exit;

    case setdocauthor:
      $sth = SELECT("* FROM libbook WHERE FileAuthor = '' AND FileType = 'fb2' LIMIT 12345");
      while ($a1=dbf($sth)) {
        Global $path, $PP; $path = array(); $PP = '';
        $xml_parser = xml_parser_create();
        xml_parser_set_option($xml_parser, XML_OPTION_CASE_FOLDING, 0);
        xml_set_element_handler($xml_parser, f1, f2);
        xml_set_character_data_handler($xml_parser, f3);
        if (!$fp = fopen($fb2 = "b/$b.fb2", "r")) 
           return "Ошибочка: не открылся файлик $fb2 ".bl($a1->BookId); 
        xml_parse($xml_parser, $d=fread($fp, 2000), 1);
        xml_parser_free($xml_parser); 
        $nick = $PP->nick;
        if (!$nick) $nick = "$PP->fn $PP->ln";
        if (!$nick) $nick = $PP->prog;
        print "<li>$a1->BookId:$nick\n"; $i++;
        if ($nick) UPDATE('libbook', "FileAuthor = \"$nick\"", "BookId = $a1->BookId");
      }
    print "<br>$i files processed";
    exit;  
    case lang:
      $sth = SELECT("BookId FROM libbook WHERE lang <> 'ru' AND FileType = 'fb2' LIMIT 22222"); 
      while ($a1=dbf($sth)) {
        $fh = fopen("b/$b.fb2", 'r');
        $Data = fread($fh, 4000);
        fclose($fh);
        if (preg_match('/<lang>(..)<.lang>/', $Data, $m)) {
          Update ('libbook', "Lang = '$m[1]'", "BookId = $a1->BookId");
        }
        $a2 = $a1;
      }
      print "<pre>";
      print_r($a2);
      print_r($m);
    exit;
    case gg:
      for ($i=2000; $i <=10000; $i++) 
       if (!Sel ("GenreId FROM libgenre WHERE BookId = $i")) 
         db_query("INSERT INTO libgenre (BookId, GenreId) VALUES ($i, 12)");
    break;      
    case da:
      $sth = SELECT("sum(1) as N, BookId, AvtorId FROM `libavtor` group by 2,3 order by 1 desc");
      while ($a1=dbf($sth)) {
        if ($a1->N < 2) return 'OK';
        db_query("DELETE FROM libavtor WHERE BookId = $a1->BookId AND AvtorId = $a1->AvtorId LIMIT ".($a1->N - 1));
      }
      exit;
    case dg:
      $sth = SELECT("sum(1) as N, BookId, GenreId FROM `libgenre` group by 2,3 order by 1 desc");
      while ($a1=dbf($sth)) {
        if ($a1->N < 2) return 'OK';
        db_query("DELETE FROM libgenre WHERE BookId = $a1->BookId AND GenreId = $a1->GenreId LIMIT ".($a1->N - 1));
      }
      exit;
    case rate:
      $sth = SELECT ("* FROM old2.liblog WHERE rate > 0");
      while ($a1=dbf($sth)) {
        $c1++;
        $a2 = S("*  FROM liblog WHERE UserId = $a1->UserId AND BookId = $a1->BookId");
        if ($a2->ID) {
          $c2++;
          if (!$a2->Rate)  //читали не оценили
            db_query("UPDATE liblog SET Rate = $a1->Rate WHERE UserId = $a1->UserId AND BookId = $a1->BookId");
        } else {
          $c3++;
          db_query("INSERT INTO liblog (UserId, BookId, Time, Rate) VALUES ($a1->UserId, $a1->BookId, '$a1->Time', $a1->Rate)");
        }     
      }      
      return "$c1 $c2 $c3";

    case 'bdesc':
      $sth = SELECT ("* FROM node join `node_revisions` using (nid) WHERE type = 'bdesc' AND body = '' AND teaser = ''");
      while ($a1 = dbf($sth)) {
       db_query('DELETE FROM {node} WHERE nid = %d', $a1->nid);
       db_query('DELETE FROM {node_revisions} WHERE nid = %d', $a1->nid);
       $r .= "<li><a href=/node/$a1->nid>$a1->title</a>";
      }
    return $r;
    case litres_genre_list:
      $xml = simplexml_load_file('http://www.litres.ru/genres_list/');
      if (!$xml) return "error loading xml"; 
      print('<pre>');
      foreach ($xml->genre as $mg) {
        $meta = $mg->attributes()->title[0];
        foreach ($mg->subgenres as $g) {
          $gname = $g->attributes()->title;
          $gcode = $g->attributes()->token;
          if (!Sel("GenreCode FROM libgenrelist WHERE GenreCode = '$gcode'")) {
            Insert (libgenrelist, 'GenreCode, GenreDesc, GenreMeta', "'$gcode', '$gname', '$meta'");
            print "\n$gcode $gname $meta ADDED";
          }  
        }
      }
      return;
    case 'xml':
      $sth = pager_query("SELECT BookId FROM libbook WHERE FileType = 'fb2' ORDER BY 1", 1000);
      while ($a1 = dbf($sth)) {
        $b = $a1->BookId;
        $xml = simplexml_load_file("b/$b.fb2");
        if (!$xml) $r .= "<li>$b <pre>".print_r(libxml_get_last_error(),1)."</pre>";
     }
    case memcache:
      print "<pre>";
      global $_memcache_statistics;
      print_r($_memcache_statistics);
      exit; 
    case libcachedel:
          dmemcache_delete("librate".arg(2), librusec);
     break;
    case libcache: 
      if (!arg(2)) {
        $hits = apc_fetch('Dolibrow Hits');
        $all = apc_fetch('Dolibrow All');
        $add = apc_fetch('Dolibrow Added');
        $per1 = ((int)($hits/($add + $hits)*100)).'%';
        $skip = $all - $add - $hits;
        $per2 = ((int)($skip/($all)*100)).'%';
        print "<p>Dolibrow hits: $hits ($per1), added $add, skip $skip ($per2), all $all";
        $hits = apc_fetch('Search hit');
        $add = apc_fetch('Search stored');
        $per = ((int)($hits/($hits+$add)*100)).'%';
        print "<p>Search hits: $hits ($per), added $add";
        exit;
      }  
      $c = cache_get(arg(2), librusec);
      if ($c) {print "Cache:<pre>"; print_r($c); exit;}
      $c = cache_get("librate".arg(2), librusec);
      if ($c) {print "RATE<pre>"; print_r($c); }
      else print "no cache for user ".arg(2);
      $c = cache_get($cid = "libwst".arg(2), librusec);
      if ($c) {print "WST<pre>"; print_r($c); }
      else print "\n\n<p>no cache for book ".arg(2);
     exit;
      
    case wst: return libWST(arg(2));
   return $r.theme('pager');
  }
  return 'не понял задачи';
}

//SELECT BookId, sum(1) AS N FROM `liblog` WHERE substr(time,3,5) = '08-12' GROUP BY 1

function f1($parser, $E, $attrs) { Global $path;  $path[] = strtolower($E);   }
function f2($parser, $E) { Global $path;  array_pop($path);}
function f3($parser, $E) {
  Global $path, $PP;  
  $p = join('/',$path);
  if ($p == 'fictionbook/description/document-info/author/nickname') $PP->nick .= $E;
  if ($p == 'fictionbook/description/document-info/author/first-name') $PP->fn .= $E;
  if ($p == 'fictionbook/description/document-info/author/last-name') $PP->ln .= $E;
  if ($p == 'fictionbook/description/document-info/program-used') $PP->prog .= $E;
}

function find_keywords(&$context) {
  $f = fopen("cache/batch.log",'a+');
  $b = $context['sandbox'];
  if (!$b) $b = 1;
  $context['finished'] = $b/140000;
//  $sth = SELECT("* FROM libbook WHERE FileType = 'fb2' AND BookId > $b AND NOT (Deleted&1) AND keywords = '' ORDER BY BookId LIMIT 50");
//  $sth = SELECT("* FROM libbook WHERE FileType = 'fb2' AND BookId = 100037 ORDER BY BookId LIMIT 50");
  $sth = SELECT("* FROM libbook LEFT JOIN libseq USING(BookId) WHERE BookId < 139999 AND FileType = 'fb2' AND ISNULL(SeqNumb) ORDER BY BookId DESC LIMIT 1200");
  while ($a1 = dbf($sth)) {
    print_r ($a1);
    $b = $a1->BookId;
    fprintf($f, " $b ");
    if ($e = parsefb2("b/$b.fb2", $b, 'head')) { fprintf($f, $e);exit;}
    Global $PD;
    print parsed;
/*
    if ($keywords = addslashes($PD->head['FICTIONBOOK/DESCRIPTION/TITLE-INFO/KEYWORDSss'])) {
      db_query("Update libbook SET keywords = '%s' WHERE BookId = $b", $keywords); 
      fprintf($f, " ($b.$keywords) ");
      $context['results'][a]++;
      $nid = Sel("nid FROM `libbnode` WHERE `BookId` = %d", $b);
      if (!$nid) {
        $node = new StdClass();
        $node->nid = 0;
        $node->type = 'bdesc';
        $node->status = 1;
        $node->title = $a1->Title; 
        node_submit($node);
        node_save($node);
        if ($nid = $node->nid) Insert(libbnode, "nid, BookId", "$nid, $b");
      } 
      if (!$nid) continue;
      $vid = Sel ("vid FROM node WHERE nid = $nid");
      foreach(split(',', $keywords) as $key) {
        $tid = Sel ("tid FROM term_data WHERE name = '%s'", $key);
        if (!$tid) {
          Insert (term_data, "vid, name, description", "3, '%s', ''", $key);
          $tid = Sel ("tid FROM term_data WHERE name = '%s'", $key);
          if (!$tid) continue;
          Insert (term_hierarchy, 'tid,	parent', "$tid, 0");
        }
        if ($nid) Insert (term_node, 'nid, tid, vid', "$nid, $tid, $node->vid");
      }
    }  
    if ($srclang = addslashes($PD->head['FICTIONBOOK/DESCRIPTION/TITLE-INFO/SRC-LANG'])) 
      db_query("Insert libsrclang (BookId, SrcLang) VALUES($b, '$%'", $srclang); 
 */
    $context['message'] = $context['sandbox'] = $b + 1;
    $context['message'] .= "/".$context['results'][a];
//    print $keywords;
    print "<pre>".print_r($PD, 1)."</pre>";

    if ($PD->SeqIds) 
      foreach($PD->SeqIds as $seq) {
        db_query ("INSERT INTO libseq(BookId, SeqId, SeqNumb, Level) VALUES ($b, $seq[0], '$seq[1]', $seq[2]) 
        ON DUPLICATE KEY UPDATE SeqNumb = '$seq[1]', Level = $seq[2]");
      }
  }

  fprintf($f, "\n");
  fclose ($f);
}
  
function find_translators(&$context) {
  return;
  $f = fopen("cache/batch.log",'a+');
  $b = $context['sandbox'];
  if (!$b) $b = 1;
  $context['finished'] = $b/138500;
  $sth = SELECT("* FROM libbook WHERE FileType = 'fb2' AND BookId > $b AND NOT (Deleted&1) ORDER BY BookId LIMIT 50");
  while ($a1 = dbf($sth)) {
    $b = $a1->BookId;
    fprintf($f, " $b ");
    if (Sel ("TranslatorId FROM libtranslator WHERE BookId = $b")) continue;
    if ($e = parsefb2("b/$b.fb2", $b, 'head')) { fprintf($f, $e);exit;}
    Global $PD;
    if ($PD->TranslatorIds)
    foreach ($PD->TranslatorIds as $aid) {
      Insert ('libtranslator', 'BookId, TranslatorId', "$b, $aid"); 
      fprintf($f, " ($b.$aid) ");
      $context['results'][a]++;
    }  
    $context['message'] = $context['sandbox'] = $b + 1;
    $context['message'] .= "/".$context['results'][a];
  }
  fprintf($f, "\n");
  fclose ($f);
}

function my_finished_callback($success, $results, $operations) {
  $f = fopen("cache/batch.log",'a+');
  fprintf($f, "success, results, operations - $success, $results, $operations");
  fclose ($f);
}

function SetActionBookId() {
  print "<pre>";  
  $sth = SELECT ("* FROM libbook LEFT JOIN libactions USING(BookId) WHERE BookId > 60000 AND ISNULL(ActionId) ORDER BY `libbook`.`BookId` DESC LIMIT 10");
  while ($a1 = dbf($sth)) {
    $id = Sel("ActionId FROM `libactions` WHERE `ActionUndo` = '' AND `BookId` = 0 AND ActionSQL like '%s' ORDER BY ActionId DESC LIMIT 1", 
                 "%$a1->FileSize%$a1->FileType%$a1->Title%$a1->Id%");
    if (!$id)             
      $id = Sel("ActionId FROM `libactions` WHERE `ActionUndo` = '' AND `BookId` = 0 AND ActionSQL like '%s' ORDER BY ActionId DESC LIMIT 1", 
                 "%$a1->FileSize%$a1->FileType%$a1->Id%");
    if ($id) {
      Update(libactions, "BookId = $a1->BookId", "ActionId=$id");
      //$q = S("* FROM libactions WHERE ActionId=$id"); print_r($q);exit;
      print "\n".bl($a1->BookId);
    } else {
      print "\n";
      print_r($a1);
      $id = Sel("ActionId FROM `libactions` WHERE `ActionUndo` = '' AND ActionSQL like '%s' ORDER BY ActionId DESC LIMIT 1", 
                 "%$a1->FileSize%$a1->FileType%$a1->Title%$a1->Id%");
      if ($id) {
        $q = S("* FROM libactions WHERE ActionId = $id");
        print "Занят под $id : ".print_r($q);
      } else {
        print "Book Fail ".bl($a1->BookId);
        print "\n"."ActionId FROM `libactions` WHERE `ActionUndo` = '' AND `BookId` = 0 AND ActionSQL like '%$a1->FileSize%$a1->FileType%$a1->Title%$a1->Id%' ORDER BY ActionId DESC ". 
                 "";
      }              
    }  
  } 
exit;
}

function libNewRead($b) {
  $html = "/tmp/qq.html";
  if ($err = newparsefb2($b, 'htm', $html)) return $err;
  readfile($html);
  exit;
}

function libCheckSchema($b1, $b2) {
  include_once drupal_get_path('module', 'librusec').'/validator.inc';
  if ($b2 > $b1) {
    $sth = SELECT("BookId FROM libbook WHERE NOT Deleted&1 AND BookId >= $b1 AND BookId <= $b2");
    while ($a1 = dbf($sth)) {
      $b = $a1->BookId;
      $r .= "<li><a href=/b/$b>$b</a> ".validate("b/$b.fb2");
    }  
  } else {
    foreach (glob("2do/*") as $fb2) 
      if ($i++>1000*($b1-1) && $i<=1000*$b1)
        $r .= "<li>$fb2 ".validate($fb2);
  }  
  return "<ol>$r</ol>";  
}


function libdb() {
  $book = new DB;
  print $book->Title(1);
  exit;
}

function CreateTorrent($b) {
  $dir = 'b.usr/';
  $fn = Sel("FileName FROM libfilename WHERE BookId = %d", $b);
  if (!$fn) return 0;
  chdir ($dir);
  if (!file_exists($fn)) return 0;
#  print `/usr/bin/maketorrent-console --target librusec.$b.torrent http://torrent.rus.ec/bt/ $fn`;
  chdir("/www");
  $torent = 'librusec.'.$b.'.torrent';
  rename($dir.$torent, 'torrents/'.$torent);
  print "  rename($dir$torent, 'torrents/'.$torent);";
}