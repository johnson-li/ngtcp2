(window.webpackJsonpBoomer=window.webpackJsonpBoomer||[]).push([["imdb.IMDbLatestPicksReactFeature"],{bUHxZ9piaJ:function(e,t,i){"use strict";Object.defineProperty(t,"__esModule",{value:!0});var n=i("rAkiKyMc0t");t.LatestPicksFeature=n.default,t.default=n.default},rAkiKyMc0t:function(e,t,i){"use strict";var n,r=this&&this.__extends||(n=function(e,t){return(n=Object.setPrototypeOf||{__proto__:[]}instanceof Array&&function(e,t){e.__proto__=t}||function(e,t){for(var i in t)t.hasOwnProperty(i)&&(e[i]=t[i])})(e,t)},function(e,t){function i(){this.constructor=e}n(e,t),e.prototype=null===t?Object.create(t):(i.prototype=t.prototype,new i)}),o=this&&this.__assign||function(){return(o=Object.assign||function(e){for(var t,i=1,n=arguments.length;i<n;i++)for(var r in t=arguments[i])Object.prototype.hasOwnProperty.call(t,r)&&(e[r]=t[r]);return e}).apply(this,arguments)};Object.defineProperty(t,"__esModule",{value:!0});var a=i("LDoPTt+kJa"),s=i("CTbtN8TX6q"),u=i("l42YtCfhhb"),p=i("ekp6onNDDg"),c=i("qDO/KKoIw4"),l=i("8yJgKchHYK"),d=i("E1DRvlOYYt"),m=i("2KSaVHM2Nk"),h=function(e){function t(t){var i=e.call(this,t)||this;return i.depthQueue=[],i.getTitleCardView=function(e,t){var n=i.props.weblabExperiments||i.context.getWeblabExperiments(),r="T1"===s(n,"IMDB_HOMEPAGE_TRAILERS_267206","C"),o="T1"===s(n,"IMDB_267239","C"),a=e.miniBuyBox?{miniBuyBoxProps:e.miniBuyBox,headingProps:e.miniBuyBox.ajaxLink?i.getMiniBuyBoxTitleHeadingProps(e):void 0}:void 0;return{href:e.poster.href,id:e.id,imdbRating:e.imdbRating,userRating:e.userRating,titleRatingCanVote:e.titleRatingCanVote,miniBuyBox:a,name:e.name,trailerLink:e.trailerLink,posterImage:i.getPosterImageView(e),overflowMenu:i.getOverflowMenuProps(e,t),watchableTitleCardMetricsContext:e.watchableTitleCardMetricsContext,trailerWeblabEnabled:r,ratingWeblabEnabled:o}},i.getPosterImageView=function(e){return{imageModel:e.poster.image,imageType:e.type}},i.sendDepthMetrics=u(i.sendDepthMetrics,5e3),i}return r(t,e),t.prototype.render=function(){var e=this,t=this.props.LatestPicksModel;return void 0===t.latestPicksItems?null:a.createElement("div",{className:"latest-picks"},a.createElement(l.default,{title:"Top picks",description:"TV shows and movies just for you"}),a.createElement(c.Shoveler,{wraps:!1},t.latestPicksItems.map(function(t,i){return a.createElement(d.default,o({key:t.id},e.getTitleCardView(t,i+1),{lazyRender:i>=6,visibilityChange:function(){return e.visibilityChange(i+1)}}))})))},t.prototype.getOverflowMenuProps=function(e,t){return{attributionItems:e.attributionItems,certificate:e.certificate,titleName:e.name,releaseYear:e.releaseYear,runtime:e.runtime,tconst:e.id,titlePageLink:e.overflowMenuHref,posterImage:this.getPosterImageView(e),overflowMenuExpandRefTag:"hm_tpks_ovflw_"+t,watchlistListItemRefTag:"hm_tpks_wl_ovflw_"+t,notInterestedListItemRefTag:"hm_tpks_not_int_ovflw_"+t}},t.prototype.getMiniBuyBoxTitleHeadingProps=function(e){return{certificate:e.certificate,releaseYear:e.releaseYear,runtime:e.runtime,posterImage:this.getPosterImageView(e),titleName:e.name,titlePageLink:e.watchOptionTitleHref}},t.prototype.visibilityChange=function(e){this.depthQueue.push(e),this.sendDepthMetrics()},t.prototype.sendDepthMetrics=function(){var e=Math.max.apply(Math,this.depthQueue),i={ref_:t.REF_TAG,pageAction:t.METRICS_CONTEXT.PAGE_ACTION+e,pt:t.METRICS_CONTEXT.PAGE_TYPE,spt:t.METRICS_CONTEXT.SUB_PAGE_TYPE,ht:t.METRICS_CONTEXT.HIT_TYPE};p.default({method:"POST",url:t.METRICS_URL,headers:{"Content-type":"application/x-www-form-urlencoded"},data:this.encodeParameters(i)}).catch(function(e){console.error("Failed to submit scroll depth metric.",e)}),this.depthQueue.splice(0,this.depthQueue.length)},t.prototype.encodeParameters=function(e){return void 0===e&&(e={}),Object.keys(e).map(function(t){return encodeURIComponent(t)+"="+encodeURIComponent(e[t])}).join("&")},t.contextType=m.SlotContext,t.METRICS_URL="/tr/",t.REF_TAG="hm_tpks",t.METRICS_CONTEXT={PAGE_ACTION:"depth-",PAGE_TYPE:"home",SUB_PAGE_TYPE:"main",HIT_TYPE:"actionOnly"},t}(a.PureComponent);t.Feature=h,t.default=h}}]);