package com.efortin.frozenbubble;

import java.io.IOException;
import java.util.List;

import org.jfedor.frozenbubble.Constants;

import com.kii.cloud.storage.Kii;
import com.kii.cloud.storage.KiiBucket;
import com.kii.cloud.storage.KiiObject;
import com.kii.cloud.storage.exception.app.BadRequestException;
import com.kii.cloud.storage.exception.app.ConflictException;
import com.kii.cloud.storage.exception.app.ForbiddenException;
import com.kii.cloud.storage.exception.app.NotFoundException;
import com.kii.cloud.storage.exception.app.UnauthorizedException;
import com.kii.cloud.storage.exception.app.UndefinedException;
import com.kii.cloud.storage.query.KiiClause;
import com.kii.cloud.storage.query.KiiQuery;
import com.kii.cloud.storage.query.KiiQueryResult;

import android.content.Context;
import android.util.Log;

public class PreferencesManager {
	private static final PreferencesManager _instance = new PreferencesManager();
	private static final String TAG = "PreferencesManager";
	private Context context = null;
	private KiiBucket userBucket = null;
	private KiiObject cloudPrefs = null;

    private PreferencesManager() {
        if (_instance != null)
            throw new IllegalStateException("Already instantiated. Use getInstance()");
    }

    public static PreferencesManager getInstance() {
        return _instance;
    }
    
    public static PreferencesManager getInstance(Context ctx) {
    	_instance.setContext(ctx);
        return _instance;
    }
	
	private KiiObject initialize() throws BadRequestException, ConflictException, ForbiddenException, NotFoundException, UnauthorizedException, UndefinedException, IOException{
		Log.d(TAG, "Initializing preferences");
		if(cloudPrefs == null)
			cloudPrefs = userBucket.object();
		cloudPrefs.set("preferences", true);
		cloudPrefs.save();
	
		return cloudPrefs;
	}
	
	public void load() throws BadRequestException, UnauthorizedException, ForbiddenException, ConflictException, NotFoundException, UndefinedException, IOException{
		Log.d(TAG, "Loading preferences");
		boolean prefsAvailable = false;
		if(userBucket == null)
			userBucket = Kii.user().bucket(Constants.BUCKET_NAME);
		// Define query conditions 
		KiiQuery query = new KiiQuery(KiiClause.equals("preferences", true));
		query.setLimit(1);
		// Query the bucket for preferences object
	    KiiQueryResult<KiiObject> result = userBucket.query(query);
	    List<KiiObject> objLists = result.getResult();
	    for (KiiObject obj : objLists) {
	      cloudPrefs = obj;
	      prefsAvailable = true;
	      Log.d(TAG, "Preferences found");
	    }
		if(!prefsAvailable)
			initialize();
	}
	
	public void save() throws BadRequestException, ConflictException, ForbiddenException, NotFoundException, UnauthorizedException, UndefinedException, IOException{
		cloudPrefs.save();
		Log.d(TAG, "Saved preferences");
	}
	
	public void saveAllField(boolean flag) throws BadRequestException, ConflictException, ForbiddenException, NotFoundException, UnauthorizedException, UndefinedException, IOException{
		cloudPrefs.saveAllFields(flag);
		Log.d(TAG, "Saved all preferences");
	}
	
	public void set(String key, String value){
		cloudPrefs.set(key, value);
		Log.d(TAG, "Saved key: " + key);
	}
	
	public void set(String key, Boolean value){
		cloudPrefs.set(key, value);
		Log.d(TAG, "Saved key: " + key);
	}
	
	public void set(String key, int value){
		cloudPrefs.set(key, value);
		Log.d(TAG, "Saved key: " + key);
	}
	
	public String getString(String key, String dflt){
		Log.d(TAG, "Loading key: " + key);
		return cloudPrefs.getString(key, dflt);
	}
	
	public Boolean getBoolean(String key, Boolean dflt){
		Log.d(TAG, "Loading key: " + key);
		return cloudPrefs.getBoolean(key, dflt);
	}
	
	public int getInt(String key, int dflt){
		Log.d(TAG, "Loading key: " + key);
		return cloudPrefs.getInt(key, dflt);
	}

	public Context getContext() {
		return context;
	}

	public void setContext(Context context) {
		this.context = context;
	}

}
