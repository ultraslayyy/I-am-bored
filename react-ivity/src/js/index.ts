let activeEffect: (() => void) | null = null;
const dependencyMap = new WeakMap();

type StateObject<T> = {
    value: T;
}

export function useState<T>(initialValue: T): StateObject<T> {
    let value = initialValue;
    
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

    return new Proxy({ value }, handler);
}

export function useEffect(fn: () => void) {
    const effect = () => {
        activeEffect = effect;
        fn();
        activeEffect = null;
    }
    effect();
}