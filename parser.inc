<?php

function pp($s) {  // ParsePrint - печать в текущий файл результат конвертора
  Global $PD;
  if (!isset($PD->noten)) $PD->noten = 0;
  if ($PD->IN['BODY']) $PD->text[$PD->noten] .= $s;
  if ($PD->IN['ANNOTATION']) $PD->ann .= $s;
}

Global $PD, $html1, $html2;
//Трансляция открывающих тэгов fb2 => html. Можно без угловых скобок - добавятся при необходимости.
$html1 = array ('EPIGRAPH' => '<blockquote class="epigraph">', 'TEXT-AUTHOR' => '<blockquote class=book><i>', 'ANNOTATION' => 'h4 class=book', 
'POEM' => '<div class=poem>', 'V' => 'div', 'EMPTY-LINE' => 'br', 'STANZA' => '<div class=stanza>', 'P' => 'p class=book', 
'SUBTITLE' => 'h5 class=book', 'CITE' => 'blockquote class=book', 'STRONG' => 'b', 'EMPHASIS' => 'i', 'STRIKETHROUGH' => 'strike');

//Трансляция закрывающих тэгов fb2 => html. Отсутствующие тут транслируются по предыдущей схеме.
$html2 = array ('EPIGRAPH' => '</blockquote>', 'TEXT-AUTHOR' => '</i></blockquote>', 'EMPTY-LINE' => '-', 'TITLE' => '</h3>', 'SECTION' => '-', 
'POEM' => '</div>', 'STANZA' => '</div>', 'V' => '</div>', 'IMAGE' => '-', 'P'=>'</p>','ANNOTATION' => '</h4>','SUBTITLE' => '</h5>', 'CITE' => '</blockquote>'); 
function libParserStart($parser, $E, $attrs) {
  Global $PD, $html1;
  if ($PD->done) return;
  $E = strtoupper($E); 
   foreach ($attrs as $key => $val) {
     $attrs[$key]='';
     $attrs[strtolower($key)] = strtolower($val);
   }  
  $PD->IN[$E]++;
  $PD->path[] = $E;
  switch($E) {
    case 'FICTIONBOOK': 
      $PD->xl = 'xmlns:l';
      foreach ($attrs as $key => $val) {
        list($s1, $s2) = split(":", $key, 2);
        if ($s2) $PD->xl = "$s2:href";
      }  
    return;
    case 'BODY':
      if ($attrs['name'] == 'notes') $PD->IN['NOTES'] = 1;
      if ($PD->format == 'head' || $PD->format == 'fbd-head') $PD->done = 1;
    return;
    case 'ANNOTATION':
     return;
    case 'BINARY': 
      if (!$PD->imagefile = $attrs['id']) {
        $PD->imagefile = 'cover.jpg';
        if (!$PD->cover) $PD->cover = "cover.jpg";
      }  
    return;  
    case 'IMAGE': 
      $img = $attrs[$PD->xl];
      if (!$img) $img = $attrs['l:href'];
//      GLOBAL $user; if ($user->uid == 1 && !$img) {print "<pre>$img\n";print_r ($attrs); print_r($PD);exit;}
      if (!$img) return;
      if (preg_match('/^#(.+)/', $img, $m)) { //картинка внутри файла
        $img = $m[1];
        if ($PD->format == 'htm' || $PD->format == 'fbd') $img = "/i/".($PD->b%100)."/$PD->b/$img";
      } 
      if ($img == $PD->cover) return; // обложку не повторяем
      if ($PD->format == 'txt') pp(" [Картинка: $img] ");
      else pp("<img border=0 style='spacing 9px;' src=\"$img\">");
      if ($PD->IN['COVERPAGE']) $PD->cover = $img;
    return;  
   case 'PUBLISH-INFO':
      $PD->seqlevel = 100;
   break; 
   case 'TITLE-INFO':
      $PD->seqlevel = 0;
   break; 
   case 'SEQUENCE':
      $PD->seqlevel++;
      $seqn = $attrs['number'];
      $seqid = AddSeqId($attrs['name']);
      $PD->SeqIds[] = array($seqid, $seqn, $PD->seqlevel);
    return;  
  }
    
//  if (!$PD->IN['BODY']) return;
  if ($PD->format == 'txt') {
    if ($E == 'EMPTY-LINE') pp("\n");
    if ($E == 'P') pp("\n   ");
    return;
  }
  switch ($E) {
    case 'A':  
      foreach ($attrs as $key => $val) {
        if ($val) {
          if ($key == 'type') $atype = $val;
          if ($key == $PD->xl || $key == 'l:href') $adest = $val;
          $la .= ' '.$key.' = "'.$val.'"';
        }  
      }  
      if (strtolower($atype) == 'note') { //ссылка на примечание 
        $PD->noten++;
        $PD->RevLink[$adest] = $PD->noten;
        $PD->notelink = 1;
      } else { // ссылка. просто ссылка.
        $PD->notelink = 0;
        pp("<a $la>");
      }
    break;
    case 'SECTION':
      if ($PD->SectionID = $attrs['id'])      
        pp("<a name=\"$PD->SectionID\"></a>"); 
    break;
    case 'TITLE':   
      if ($PD->IN['SECTION'] && !$PD->SectionID) {
          $PD->tocn++;
          pp("<a name=t$PD->tocn></a><h3 class=book>");
          $PD->TOC .= "<li>";
          for ($i = 1; $i < $PD->IN['SECTION']; $i++) $PD->TOC .= ' &nbsp; ';
          $PD->TOC .= "<a href=#t$PD->tocn>";
      } else pp('<h3 class=book>');  
    break;
    case 'P':
      if (!$PD->IN['TITLE'])
        pp("<p class=book>");
    break;
    default:
      if ($t = $html1[$E]) {
        if ($t[0] != '<') $t = "<$t>";
        pp ($t);
      }  else { //неизвестный науке тэг, оставляем как есть со всеми параметрами
        pp("<$E");
        foreach ($attrs as $key => $val) if ($val) pp (' '.$key.' = "'.$val.'" ');
        pp('>');
      }  
     break;  
  }
}

function libParserEnd($parser, $E) {
  Global $PD, $html1, $html2;
  if ($PD->done) return;
  array_pop($PD->path);
  switch ($E = strtoupper($E)) {
    case 'AUTHOR':
    if ($PD->path[2] == 'TITLE-INFO') { 
      $pp = 'FICTIONBOOK/DESCRIPTION/TITLE-INFO/AUTHOR/';
      $PD->AvtorIds[] = AddAvtorId($PD->head[$pp.'FIRST-NAME'], $PD->head[$pp.'MIDDLE-NAME'], $PD->head[$pp.'LAST-NAME'], $PD->head[$pp.'NICK-NAME']);
      unset($PD->head[$pp.'FIRST-NAME'], $PD->head[$pp.'MIDDLE-NAME'], $PD->head[$pp.'LAST-NAME'], $PD->head[$pp.'NICK-NAME']);
    } 
    break;
    case 'TRANSLATOR':
    if ($PD->path[2] == 'TITLE-INFO') { 
      $pp = 'FICTIONBOOK/DESCRIPTION/TITLE-INFO/TRANSLATOR/';
      $PD->TranslatorIds[] = AddAvtorId($PD->head[$pp.'FIRST-NAME'], $PD->head[$pp.'MIDDLE-NAME'], $PD->head[$pp.'LAST-NAME'], $PD->head[$pp.'NICK-NAME']);
      unset($PD->head[$pp.'FIRST-NAME'], $PD->head[$pp.'MIDDLE-NAME'], $PD->head[$pp.'LAST-NAME'], $PD->head[$pp.'NICK-NAME']);
    } 
    break;
    case 'BINARY':
      $subdir = 'i/'.($PD->b % 100);
      if (!file_exists($subdir) && !mkdir($subdir)) return;
      $dir = $subdir.'/'.$PD->b;
      if (!file_exists($dir) && !mkdir($dir)) return;
      $fh = fopen($dir.'/'.$PD->imagefile, 'w');
      fwrite($fh, base64_decode($PD->data));
      fclose($fh);
      $PD->data = '';
    return;
    case 'FICTIONBOOK':
    return;
    case 'BODY':
      if ($PD->IN['NOTES']) $PD->IN['NOTES'] = '';
      $PD->IN['BODY'] = '';
    return;
    case 'HEAD':
      if ($PD->format == 'head' || $PD->format == 'fbd-head')
        $PD->done = 1;
    case 'ANNOTATION':
      if ($PD->format == 'annotation')
        $PD->done = 1;
  }
  $PD->IN[$E]--;
  if ($PD->format == 'txt') return;
  if ($E == 'SECTION' && $PD->SectionID) {    
    $noten = $PD->RevLink[$adest='#'.$PD->SectionID];
//    global $user; if (!$noten && $user->uid == 1) {print "<pre>"; print_r ($PD); exit;}
    pp(" <small>(<a href=#r$noten>обратно</a>)</small>");  
    $note = addslashes($PD->note);
    $PD->text[$noten-1] .= "<sup><a name=r$noten><a href=\"$adest\" title=\"".str_replace('"', '&quot;', $PD->note)."\">";
  }
  if ($E == 'A' && $PD->notelink) pp('</sup>'); 
  if ($E == 'TITLE' && $PD->IN['SECTION'] && !$PD->SectionID) $PD->TOC .= "</a>";
  if ($E == 'P' && $PD->IN['TITLE']) return;
  $t = $html2[$E];
  if ($t == '-' || $t == 'BR') return;
  if (!$t) $t = $html1[$E];
  if (!$t) $t = $E;
  if ($t != "\n" && $t[0] != '<') $t = "</$t>";
  pp ($t);
}

function libParserData($parser, $E) {
  Global $PD;
  if ($PD->done) return;

  if ($PD->IN['BINARY']) {
    $PD->data .= $E;
    return;
  }  
  $E = str_replace('>','&gt;',$E);
  $E = str_replace('<','&lt;',$E);
  if ($PD->IN['NOTES']) {
    if ($PD->IN['TITLE']) $PD->note = '';  
    else $PD->note .= $E;  
  } else {
    if ($PD->IN['TITLE'] && $PD->IN['SECTION']) 
    $PD->TOC .= $E;
  }  
  if ($PD->IN['DESCRIPTION']) {
    $p = join('/',$PD->path);
    if ($PD->head[$p] || $E != ' ' && $E != "\n")
    if (strstr($p, 'AUTHOR')) {
      $PD->head[$p] = $E; 
    } elseif (strstr($p, 'ANNOTATION') ) {
      $PD->head[$p] .= $E; 
     } else {
      if ($PD->head[$p] && $E != ' ' && !$PD->IN['BOOK-TITLE']) $PD->head[$p] .= ',';
      $PD->head[$p] .= $E; 
    }  
  }  
  if ($PD->format == 'txt') $E = rtrim($E);
  pp($E);
}

function libRead($b) {
  $filetype = Sel("FileType FROM libbook WHERE BookId = $b");
  if ($filetype != 'fb2') $fbd = libGetFBD("b.usr/".Sel("FileName FROM libfilename WHERE BookId = $b"));
  else $fb2 = "b/$b.fb2";
  $html = "b.htm/$b.htm";
  if (SuperUser() || !file_exists($html)) {
    $key = sem_get(14, 30);
    if (!sem_acquire($key)) return;
    if (SuperUser() || !file_exists($html)) {     
      if ($fb2) {
        $err = parsefb2($fb2, $b, 'htm', $html);
      } else {
        $err = parsefb2($fbd, $b, 'fbd');
        touch($html);
      }  
      if (!$err) {
        Global $PD;
        db_query("INSERT INTO libcache (BookId, TOC, Cover) 
              VALUES (%d, '%s', '%s') ON DUPLICATE KEY UPDATE TOC='%s', Cover='%s'", $b, $PD->TOC, $PD->cover, $PD->TOC, $PD->cover);
        if ($PD->ann && !Sel("nid FROM node WHERE type = 'bdesc' AND title = $b") && !Sel("nid FROM libbnode WHERE BookId = $b")) {
          $body = addslashes($ann = $PD->ann);
          $nid = max (Sel ("MAX(nid) FROM node"), Sel ("MAX(nid) FROM node_revisions")) + 1;
          $vid = max (Sel ("MAX(vid) FROM node"), Sel ("MAX(vid) FROM node_revisions")) + 1;
          $now = time();
          db_query("Insert into node (nid, vid, type, title, uid, status, created, changed, comment, promote, moderate, sticky)
                    VALUES ($nid, $vid, 'bdesc', $b, 5, 1,  $now, $now,     2, 0, 0, 0)");  
          db_query("Insert into node_revisions (nid, vid, uid, title, body, timestamp, format)
                    VALUES ($nid, $vid, 5, $b, '$body', $now, 1)");
         }           
      }
    }
    sem_release($key);
  } 
  return $err.file_get_contents($html);
}
  
function parsefb2($fb2, $b, $format, $tofile = '') {
  unset ($GLOBALS['PD']);
  Global $PD; 
  $PD->format = $format;
  $PD->b = $b;
  $xml_parser = xml_parser_create();
  xml_parser_set_option($xml_parser, XML_OPTION_CASE_FOLDING, 0);
  xml_set_element_handler($xml_parser, libParserStart, libParserEnd);
  xml_set_character_data_handler($xml_parser, libParserData);
  if ($format == 'fbd-head' || $format == 'fbd') {
    xml_parse($xml_parser, $fb2);
  } else {  
    if (!$fp = fopen($fb2, "r")) return "Ошибочка: не открылся файлик $fb2"; 
    while (($data = fread($fp, 40960)) && xml_parse($xml_parser, $data) && !$PD->done && !feof($fp));
  }  
  xml_parser_free($xml_parser); 
  if (!$tofile) return;
  
  if (!$fh = fopen($tofile, 'w')) return "Не могу создать файлик $tofile";
  if ($PD->format == 'txt') {
    $PD->text[++$PD->noten] = "\n\nВзято из Либрусека, http://lib.rus.ec/b/$PD->b\n";
    for($i = 0; $i <= $PD->noten; $i++) 
      fwrite($fh, iconv("UTF-8", "Windows-1251//TRANSLIT", $PD->text[$i]));
  } else {     
    if ($PD->format == 'html') fwrite($fh, 
      "<html><head><meta http-equiv='Content-Type' content='text/html; charset=utf8'>\n".
      file_get_contents(drupal_get_path('module', 'librusec').'/book.css').
      "<title>".Sel("Title FROM libbook WHERE BookId = $PD->b")." - Либрусек</title>\n</head>\n<body>\n");
    for($i = 0; $i <= $PD->noten; $i++) 
      fwrite($fh, $PD->text[$i]);
    if ($PD->TOC) 
      fwrite($fh, "<br><h3>Оглавление</h3>".$PD->TOC);
    if ($PD->AvtorIds[0] == 5329) 
      fwrite($fh, "\n<br>\nСайт автора <a href=http://lleo.aha.ru>http://lleo.aha.ru</a>\n");  
    if ($PD->format == 'html') 
      fwrite($fh, "\n<hr><div align=right>Взято из Либрусека, <a href=http://lib.rus.ec/b/$PD->b>lib.rus.ec</a></div>\n</body></html>");
  }
  fclose($fh);  
}

function parse_fb2_document_info($f) {
  if ($err = parsefb2($f, 0, 'head')) return $err;
  Global $PD;
  foreach ($PD->head as $i => $value) 
    if ($value)
      $r .= "<li>".strtolower($i)." $value";
  return "<ul>$r</ul>"; 
}

function MakeFile($b, $f, $dtp, $r) {
  $fb2 = "b/$b.fb2";
  switch ($dtp) {
    case 'txt':
    case 'html':
      return parsefb2($fb2, $b, $dtp, $r);
    case 'rtf':
     chdir ('conv');
     exec ('perl fb2_2_rtf.pl "../'.$fb2.'" FB2_2_rtf.xsl "../'.$r.'"');
     chdir ('..');
    return;
    case 'epub':
      chdir('conv/fb2docbook');
      exec('python btransformer.py --config btconfig_default.xml --epub /www/'.$fb2.' "/www/'.$r.'"');
     chdir ('../..');
    return;
    case 'pdb':
     exec ('conv/iSiloXC "'.$fb2.'" "'.$r.'"');
    return;
    case 'rb':
    case 'pdf':
  }  
}

function libGenerateFB2($b, $fb2) {
return '';
            foreach($xml->description->{'custom-info'} as $q) {
              if ($q->attributes()->{'into-type'} == 'Librusec BookId') {
                $q[0]=$b;
                $BookIdSet = 1;
                break;
              }  
            } 
            if (!$BookIdSet)
              $xml->description->addChild('custom-info', $b)->addAttribute('into-type', 'Librusec BookId');
            $titinfo = $xml->description->{'title-info'};
            $a1 = dbf(SELECT("* FROM libbook WHERE BookId = $b"));
            $titinfo->{'book-title'} = $a1->Title;
            if ($titinfo->lang) $titinfo->lang = $a1->Lang;
            else $titinfo->addChild('lang', $a1->Lang);
            unset($titinfo->genre);
            $sth = SELECT ("GenreCode FROM libgenre JOIN libgenrelist USING(GenreId) WHERE BookId = $b");
            while ($a2 = dbf($sth)) $titinfo->genre[] = $a2->GenreCode;
            foreach($titinfo->author as $a) {
              $an = $a->{'first-name'}.$a->{'last-name'};
              if ($a->email) $email[$an] = $a->email.'';
              if ($a->{'home-page'}) $homepage[$an] = $a->{'home-page'}.'';
            }
            unset($titinfo->author);
            $sth = SELECT ("* FROM libavtorname JOIN libavtor USING(AvtorId) WHERE BookId = $b");
            while ($a2 = dbf($sth)) {
              $aa = $titinfo->addChild('author');
              $an = $a2->FirstName.$a2->LastName;
              if (!$a2->Email AND $email[$an]) $a2->Email = $email[$an];
              if (!$a2->Homepage AND $homepage[$an]) $a2->Homepage = $homepage[$an];
              $tag = array(FirstName => 'first-name',  MiddleName => 'middle-name', LastName => 'last-name',  NickName => 'nickname', 
                           Email => 'email', Homepage => 'home-page');
              foreach ($tag as $name=>$t) if ($a2->$name)  $aa->addChild($t, $a2->$name);
            }
            $ann = ReadAnnotation($b);
            if ($ann AND $ann !== 'нет') $ann = simplexml_load_string($ann);
            else unset($ann);
            if ($ann) {
             unset($titinfo->annotation);
             $titinfo->addChild('annotation')->addChild('p',$ann);
            } else {
              unset($titinfo->annotation);
            }   
            if ($a1->Year AND $titinfo->date != $a1->Year) {
              unset($titinfo->date);
              $titinfo->addChild('date', $a1->Year)->addAttribute('value', $a1->Year);
            }  
            unset($titinfo->sequence);
            
            $sth = SELECT ("* FROM libseq WHERE BookId = %d ORDER BY Level", $b); 
            while ($a1 = dbf($sth)) {
              $seq = $titinfo->addChild('sequence');
              $seq->addAttribute('name', Sel("SeqName FROM libseqname WHERE SeqId = $a1->SeqId"));
              $seq->addAttribute('number', $a1->SeqNumb);
            }  
            $xml->asXML($fb2);
          } 



function libGetFBD($f) {
  if (pathinfo($f, PATHINFO_EXTENSION) != 'zip') return '';
  if (!is_resource($zip = zip_open($f))) return '';
  while($zip_entry = zip_read($zip)) 
    if (pathinfo(zip_entry_name($zip_entry), PATHINFO_EXTENSION) == 'fbd') 
      return zip_entry_read($zip_entry, zip_entry_filesize($zip_entry));
  return '';    
}
