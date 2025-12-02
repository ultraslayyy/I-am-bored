let activeEffect: (() => void) | null = null;
const dependencyMap = new WeakMap();

const globalHookState: any[] = [];
let hookIndex = 0;

type StateObject<T> = {
    value: T;
}

export function useState<T>(initialValue: T): StateObject<T> {
    const currentHookIndex = hookIndex;
    hookIndex++;

    if (globalHookState[currentHookIndex] !== undefined) {
        return globalHookState[currentHookIndex] as StateObject<T>;
    }

    const stateContainer = { value: initialValue }
    
    const handler = {
        get(target: any, key: string | symbol) {
            if (activeEffect) {
                let propertyEffects = dependencyMap.get(target);
                if (!propertyEffects) {
                    propertyEffects = new Map();
                    dependencyMap.set(target, propertyEffects);
                }
                let effectsSet = propertyEffects.get(key);
                if (!effectsSet) {
                    effectsSet = new Set();
                    propertyEffects.set(key, effectsSet);
                }
                effectsSet.add(activeEffect);
            }
            return target[key];
        },
        set(target: any, key: string | symbol, newValue: any) {
            if (target[key] !== newValue) {
                target[key] = newValue;

                const propertyEffects = dependencyMap.get(target)?.get(key);
                if (propertyEffects) { // @ts-ignore
                    propertyEffects.forEach(effect => effect());
                }
            }
            return true;
        }
    }

    const proxiedState = new Proxy(stateContainer, handler) as StateObject<T>;
    globalHookState[currentHookIndex] = proxiedState;

    return proxiedState;
}

export function useEffect(fn: () => void, dependencyArray?: any[]) {
    const effect = () => {
        hookIndex = 0;

        activeEffect = effect;
        fn();
        activeEffect = null;
    }
    effect();
}

export function useCallback<T extends Function>(callback: T, dependencies: any[]): T {
    const currentHookIndex = hookIndex;
    hookIndex++;

    if (globalHookState[currentHookIndex] === undefined) {
        globalHookState[currentHookIndex]= {
            fn: callback,
            deps: dependencies
        }
        return callback;
    }

    const cachedHook = globalHookState[currentHookIndex];
    const previesDeps = cachedHook.deps;
    let dependenciesChanged = previesDeps.length !== dependencies.length;

    if (!dependenciesChanged) {
        for (let i = 0; i < dependencies.length; i++) {
            if (dependencies[i] !== previesDeps[i]) {
                dependenciesChanged = true;
                break;
            }
        }
    }

    if (dependenciesChanged) {
        cachedHook.fn = callback;
        cachedHook.deps = dependencies;
        return callback;
    } else {
        return cachedHook.fn as T;
    }
}