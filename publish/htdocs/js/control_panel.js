/**
 * Created by Frankie.W on 2018/11/24.
 */

function LoadModel (){
    var modelName = document.getElementById("tmodelName").value;
    var modelUrl="./assets/name/name.model.json";
    modelUrl = modelUrl.replace(/name/g, modelName);
    console.log(modelUrl);
    loadlive2d("live2dCanvas", modelUrl);
}

function init(){
    LoadModel();
}
