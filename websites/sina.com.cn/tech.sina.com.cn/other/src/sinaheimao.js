(function(e){function n(e,n){return e.replace(/\$\$(.*?)\$\$/g,function(e,t){return n[t]})}function t(e){function n(e){var n=e.jsonp,a=document.getElementsByTagName("head")[0];e.data.callback=n;var r=t(e.data),i=document.createElement("script");a.appendChild(i),window[n]=function(t){a.removeChild(i),clearTimeout(i.timer),window[n]=null,e.success&&e.success(t)},i.src=e.url+"?"+r,e.time&&(i.timer=setTimeout(function(){window[n]=null,a.removeChild(i),e.error&&e.error({message:"超时"})},time))}function t(e){var n,t=[];for(n in e)t.push(encodeURIComponent(n)+"="+encodeURIComponent(e[n]));return t.push("dpc=1"),t.join("&")}e=e||{},e.data=e.data||{};var a=e.jsonp?n(e):a(e)}if(e.addEventListener){var a='<div class="heimao-item"><p class="heimao-p1"><a href="$$url$$" target="_blank">$$company$$</a></p><p class="heimao-p2"><a href="$$url$$" target="_blank">$$title$$</a></p></div>',r=e.getElementById("heimao_title");t({url:"//tousu.sina.com.cn/api/int_prom/get",jsonp:"heimaosuccess",data:{channel:5},success:function(t){var i=e.querySelectorAll(".heimao-lists"),o=t.result.data,s=o.list;r.innerText=o.slogan,r.setAttribute("href",o.slink);var c=s||o,l=c.length,u="";if(l>0){for(var m=0;m<2;m++)c[m]&&(u+=n(a,c[m]));i[0].innerHTML=u,i.length>0&&(i[1].innerHTML=heimao_str)}},error:function(e){}})}})(document);